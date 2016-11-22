/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Main application control functions
 *
 */

#include <ad985x.h>
#include <stdio.h>
#include <stdarg.h>
#include <iodev.h>
#include "buildid.h"
#include "radio3.h"
#include "fmeter.h"
#include "board.h"
#include "datalink.h"
#include "adc.h"
#include "cortexm/ExceptionHandlers.h"

#define MAX_PAYLOAD_SIZE        16
#define TICKS_PER_SECOND        100
#define MS_PER_TICK                10

#define DEVICE_RESET            0x000
#define DEVICE_INFO                0x001
#define DEVICE_STATE            0x002

#define VFO_GET_FREQ            0x008
#define VFO_SET_FREQ            0x009
#define VFO_ERROR            0x00f

#define LOGPROBE_GET            0x010
#define LOGPROBE_INFO            0x016
#define LOGPROBE_ERROR        0x017

#define LINPROBE_GET            0x018
#define LINPROBE_ERROR        0x01f

#define CMPPROBE_GET            0x020
#define CMPPROBE_ERROR        0x027

#define FMETER_GET                0x028
#define FMETER_ERROR            0x02f

#define PROBES_GET                0x030
#define PROBES_START_SAMPLING    0x031
#define PROBES_STOP_SAMPLING    0x032

#define ANALYSER_REQUEST        0x040
#define ANALYSER_STOP            0x041
#define ANALYSER_STATE            0x042
#define ANALYSER_DATA            0x043

#define ANALYSER_HEADER_SIZE    12
#define ANALYSER_MAX_STEPS        1000
#define ANALYSER_MAX_SERIES        2

#define ERROR_INVALID_FRAME        0x3fe
#define LOG_MESSAGE                0x3ff

#define STDIO_BUF_SIZE            128

volatile uint32_t currentTime = 0;

struct Complex {
    uint16_t value;
    uint16_t phase;
};

struct DeviceState {
    uint8_t padding;
    uint8_t probesSampling;
    uint16_t samplingPeriodMs;
    uint32_t timeMs;
};

struct LogProbeInfo {
    char name[16];
    uint16_t minValue;
    uint16_t maxValue;
    int16_t minDBm;
    int16_t maxDBm;
};

struct VfoInfo {
    char name[16];
    uint32_t minFreq;
    uint32_t maxFreq;
};

struct FMeterInfo {
    char name[16];
    uint32_t minFreq;
    uint32_t maxFreq;
};

struct VnaInfo {
    char name[16];
};

struct DeviceInfo {
    char name[16];
    char buildId[32];
    struct VfoInfo vfo;
    struct FMeterInfo fMeter;
    struct LogProbeInfo logProbe;
    struct VnaInfo vnaInfo;
};

struct Probes {
    uint16_t logarithmic;
    uint16_t linear;
    struct Complex complex;
    uint32_t fmeter;
};

struct AnalyserRequest {
    uint32_t freqStart;
    uint32_t freqStep;
    uint16_t numSteps;
    uint16_t stepWaitMs;
    uint16_t source;
};

enum {
    LOG_PROBE = 0, LIN_PROBE = 1, VNA = 2
} AnalyserSource;

struct AnalyserData {
    uint32_t freqStart;
    uint32_t freqStep;
    uint16_t numSteps;
    uint16_t source;
    uint16_t data[ANALYSER_MAX_SERIES * (ANALYSER_MAX_STEPS + 1)];
};

static struct DeviceInfo deviceInfo = {
        .name = "radio3-stm32-md",
        .vfo = {.name = "DDS AD9851", .minFreq = 0L, .maxFreq = 70000000L},
        .fMeter = {.name = "STM32@72MHz", .minFreq = 0L, .maxFreq = 35000000L},
        .logProbe = {
                .name = "AD8307",
                .minValue = 372, .maxValue = 3351, // 0.3 V - 2.7 V
                .minDBm = -74, .maxDBm = 16 // -75 dBm - 16 dBm
        },
        .vnaInfo = {.name = "AD8302"}
};

static volatile struct DeviceState deviceState = {
        .probesSampling = 0,
        .samplingPeriodMs = 200,
        .timeMs = 0
};

enum {
    ANALYSER_IDLE = 0, ANALYSER_BUSY = 1, ANALYSER_INVALID_REQUEST = 2
} AnalyserState;

static uint8_t analyserState = {ANALYSER_IDLE};
static struct AnalyserData analyserData;
static char stdioBuf[STDIO_BUF_SIZE];

static void waitMs(uint16_t ms) {
    uint32_t t = currentTime + ms;
    while (currentTime < t) {}
}

static void sendData(uint16_t command, void *payload, uint16_t size) {
    datalink_writeFrame(command, payload, size);
    waitMs(50);
}

inline static void sendError(uint16_t command) {
    sendData(command, 0, 0);
}

static void setAndSendAnalyserState(uint8_t newState) {
    analyserState = newState;
    sendData(ANALYSER_STATE, &analyserState, sizeof(analyserState));
}

static uint16_t calculateAnalyserDataSteps() {
    return analyserData.source == VNA ? (analyserData.numSteps + 1) * 2 : (analyserData.numSteps + 1);
}

static void sendAnalyserData(void) {
    sendData(ANALYSER_DATA, &analyserData, ANALYSER_HEADER_SIZE + calculateAnalyserDataSteps() * sizeof(uint16_t));
}

static void logPrintf(char *format, ...) {
    va_list va;
    va_start(va, format);
    int len = vsnprintf(stdioBuf, STDIO_BUF_SIZE, format, va);
    va_end(va);
    sendData(LOG_MESSAGE, stdioBuf, len);
}

static void performAnalysis(struct AnalyserRequest *req) {
    analyserData.freqStart = req->freqStart;
    analyserData.freqStep = req->freqStep;
    analyserData.numSteps = req->numSteps;
    analyserData.source = req->source;

    uint32_t freq = analyserData.freqStart;
    ad985x_setFrequency(freq);

    // wait for initial stabilization
    volatile int w = 50000;
    while (w) { w--; }

    uint16_t numSteps = calculateAnalyserDataSteps();
    uint16_t step = 0;
    while (step <= numSteps) {
        ad985x_setFrequency(freq);

        // wait until frequency and DUT response stabilizes before next measurement
        volatile int w = 2000;
        while (w) { w--; }

        switch (analyserData.source) {
            case LOG_PROBE:
                analyserData.data[step++] = adc_readLogarithmicProbe();
                break;
            case LIN_PROBE:
                analyserData.data[step++] = adc_readLinearProbe();
                break;
            case VNA:
                analyserData.data[step++] = adc_readVnaGainValue();
                analyserData.data[step++] = adc_readVnaPhaseValue();
        }
        freq += analyserData.freqStep;
    }

    ad985x_setFrequency(0);
}

static void cmdGetDeviceInfo(void) {
    RCC_ClocksTypeDef ctd;
    RCC_GetClocksFreq(&ctd);
    datalink_writeFrame(DEVICE_INFO, &deviceInfo, sizeof(deviceInfo));
}

static void cmdGetDeviceState(void) {
    struct DeviceState ds = deviceState;
    ds.timeMs = currentTime;
    datalink_writeFrame(DEVICE_STATE, &ds, sizeof(ds));
}

static void cmdSetVfoFrequency(uint8_t *payload) {
    ad985x_setFrequency(*((uint32_t *) payload));
}

static void cmdAnalyserStart(uint8_t *payload) {
    if (analyserState != ANALYSER_BUSY) {
        struct AnalyserRequest *req = (struct AnalyserRequest *) payload;
        uint32_t freqEnd = req->freqStart + (req->freqStep * req->numSteps);
        logPrintf("start analyzer from %lu to %lu in %d steps, source: %d", req->freqStart, freqEnd, req->numSteps,
                  req->source);
        if (req->numSteps > 0 && req->numSteps <= ANALYSER_MAX_STEPS &&
            req->freqStart >= deviceInfo.vfo.minFreq && freqEnd <= deviceInfo.vfo.maxFreq) {
            setAndSendAnalyserState(ANALYSER_BUSY);
            performAnalysis(req);
            sendAnalyserData();
            setAndSendAnalyserState(ANALYSER_IDLE);
        } else {
            setAndSendAnalyserState(ANALYSER_INVALID_REQUEST);
        }
    }
}

static void cmdGetVfoFrequency(void) {
    uint32_t frequency = ad985x_getFrequency();
    datalink_writeFrame(VFO_GET_FREQ, &frequency, sizeof(frequency));
}

static void cmdSampleFMeter(void) {
    uint32_t frequency = fmeter_read();
    datalink_writeFrame(FMETER_GET, &frequency, sizeof(frequency));
}

static void cmdSampleLogarithmicProbe(void) {
    uint16_t value = adc_readLogarithmicProbe();
    datalink_writeFrame(LOGPROBE_GET, &value, sizeof(value));
}

static void cmdSampleLinearProbe(void) {
    uint16_t value = adc_readLinearProbe();
    datalink_writeFrame(LINPROBE_GET, &value, sizeof(value));
}

inline static struct Complex readVnaProbe(void) {
    struct Complex c = {
            .value = adc_readVnaGainValue(),
            .phase = adc_readVnaPhaseValue()
    };
    return c;
}

inline static struct Probes readAllProbes(void) {
    struct Probes p = {
            .logarithmic = adc_readLogarithmicProbe(),
            .linear = adc_readLinearProbe(),
            .complex = readVnaProbe(),
            .fmeter = fmeter_read()
    };
    return p;
}

static void cmdSampleComplexProbe(void) {
    struct Complex gp = readVnaProbe();
    datalink_writeFrame(CMPPROBE_GET, &gp, sizeof(gp));
}

static void cmdSampleProbes(void) {
    struct Probes data = readAllProbes();
    datalink_writeFrame(PROBES_GET, &data, sizeof(data));
}

static void systick_init(void) {
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);
}

void SysTick_Handler(void) {
    static uint8_t fmeter_timebase_counter = TICKS_PER_SECOND;

    currentTime += MS_PER_TICK;
    if (!(--fmeter_timebase_counter)) {
        fmeter_timebase();
        fmeter_timebase_counter = TICKS_PER_SECOND;
    }
}

void radio3_init() {
    snprintf(deviceInfo.buildId, 31, "%s", BUILD_ID);
    ad985x_init();
    ad985x_setFrequency(0);
    systick_init();
    fmeter_init();
    adc_init();
    logPrintf("started!");
}

static void handleIncomingFrame(void) {
    static struct datalink_frame frame;
    static uint8_t payload[MAX_PAYLOAD_SIZE];

    datalink_readFrame(&frame, payload, MAX_PAYLOAD_SIZE);
    if (datalink_error()) { return; }

    switch (frame.command) {
        case DEVICE_RESET:
            radio3_init();
            break;

        case DEVICE_INFO:
            cmdGetDeviceInfo();
            board_ledYellow(0);
            break;

        case DEVICE_STATE:
            cmdGetDeviceState();
            break;

        case VFO_GET_FREQ:
            cmdGetVfoFrequency();
            break;

        case VFO_SET_FREQ:
            cmdSetVfoFrequency(payload);
            break;

        case LOGPROBE_GET:
            cmdSampleLogarithmicProbe();
            break;

        case LINPROBE_GET:
            cmdSampleLinearProbe();
            break;

        case CMPPROBE_GET:
            cmdSampleComplexProbe();
            break;

        case FMETER_GET:
            cmdSampleFMeter();
            break;

        case PROBES_GET:
            cmdSampleProbes();
            break;

        case PROBES_START_SAMPLING:
            deviceState.probesSampling = 1;
            break;

        case PROBES_STOP_SAMPLING:
            deviceState.probesSampling = 0;
            break;

        case ANALYSER_REQUEST:
            cmdAnalyserStart(payload);
            break;

        default:
            sendError(ERROR_INVALID_FRAME);
            board_ledYellow(1);
    }
}

static void handleDataSampling(void) {
    static uint32_t nextSamplingTimeMs = 0;

    if (currentTime > nextSamplingTimeMs) {
        nextSamplingTimeMs = currentTime + deviceState.samplingPeriodMs;

        if (deviceState.probesSampling) {
            cmdSampleProbes();
            waitMs(10);
        }
    }
}

void radio3_start() {
    while (1) {
        handleDataSampling();
        board_ledOnModule(1);
        board_ledGreen(1);
        if (datalink_isIncomingData()) {
            board_ledOnModule(0);
            board_ledGreen(0);
            handleIncomingFrame();
        }
    }
}

void main(void) {
    board_init();
    iodev_init();
    datalink_init();
    radio3_init();
    radio3_start();
}
