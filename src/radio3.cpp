/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Main application control functions
 *
 */

#include <stdio.h>
#include <stdarg.h>

#include "ad985x.h"
#include "iodev.h"
#include "buildid.h"
#include "radio3.h"
#include "fmeter.h"
#include "board.h"
#include "datalink.h"
#include "adc.h"
#include "cortexm/ExceptionHandlers.h"

static constexpr auto MAX_PAYLOAD_SIZE = 16;
static constexpr auto TICKS_PER_SECOND = 100;
static constexpr auto MS_PER_TICK = 10;

#define DEVICE_RESET            0x000
#define DEVICE_INFO             0x001
#define DEVICE_STATE            0x002

#define VFO_GET_FREQ            0x008
#define VFO_SET_FREQ            0x009
#define VFO_ERROR               0x00f

#define LOGPROBE_GET            0x010
#define LOGPROBE_INFO           0x016
#define LOGPROBE_ERROR          0x017

#define LINPROBE_GET            0x018
#define LINPROBE_ERROR          0x01f

#define CMPPROBE_GET            0x020
#define CMPPROBE_ERROR          0x027

#define FMETER_GET              0x028
#define FMETER_ERROR            0x02f

#define PROBES_GET              0x030
#define PROBES_START_SAMPLING   0x031
#define PROBES_STOP_SAMPLING    0x032
#define SET_DDS_RELAY_VFO       0x033
#define SET_DDS_RELAY_VNA       0x034

#define ANALYSER_REQUEST        0x040
#define ANALYSER_STOP           0x041
#define ANALYSER_DATA           0x042

#define ANALYSER_HEADER_SIZE    12
#define ANALYSER_MAX_STEPS      1000
#define ANALYSER_MAX_SERIES     2

#define ERROR_INVALID_FRAME     0x3fe
#define LOG_MESSAGE             0x3ff

#define STDIO_BUF_SIZE          128

volatile uint32_t currentTime = 0;

enum class VfoType : uint8_t { NONE, AD9850, AD9851 };

enum class AnalyserDataSource : uint8_t { LOG_PROBE, LIN_PROBE, VNA };

enum class AnalyserState : uint8_t { READY, PROCESSING, INVALID_REQUEST };

enum class DdsOut : uint8_t { VFO, VNA };

struct Complex {
    uint16_t value;
    uint16_t phase;
} __packed;

struct DeviceState {
    uint8_t probesSampling;
    uint16_t samplingPeriodMs;
    uint32_t timeMs;
    AnalyserState analyser;
    DdsOut ddsOut;
} __packed;

struct VfoInfo {
    VfoType type;
    uint32_t minFreq;
    uint32_t maxFreq;
} __packed;

struct DeviceInfo {
    const char name[16];
    const char buildId[32];
    VfoInfo vfo;
} __packed;

struct Probes {
    uint16_t logarithmic;
    uint16_t linear;
    Complex complex;
    uint32_t fMeter;
} __packed;

struct AnalyserRequest {
    uint32_t freqStart;
    uint32_t freqStep;
    uint16_t numSteps;
    AnalyserDataSource source;
} __packed;

struct AnalyserData {
    uint32_t freqStart;
    uint32_t freqStep;
    uint16_t steps;
    AnalyserDataSource source;
    uint16_t data[ANALYSER_MAX_SERIES * (ANALYSER_MAX_STEPS + 1)];
} __packed;

static DeviceInfo deviceInfo = {
        "radio3-stm32-md",
        BUILD_ID,
        { VfoType::AD9851, 0L, 70000000L },
};

static DeviceState deviceState = { 0, 200, 0, AnalyserState::READY, DdsOut::VFO};
static AnalyserData analyserData;
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

static uint16_t calculateAnalyserDataSteps() {
    return (uint16_t) (analyserData.source == AnalyserDataSource::VNA ? (analyserData.steps + 1) * 2 : (analyserData.steps + 1));
}

static void sendAnalyserData() {
    sendData(ANALYSER_DATA, &analyserData, ANALYSER_HEADER_SIZE + calculateAnalyserDataSteps() * sizeof(uint16_t));
}

static void logPrintf(const char *format, ...) {
    va_list va;
    va_start(va, format);
    int len = vsnprintf(stdioBuf, STDIO_BUF_SIZE, format, va);
    va_end(va);
    sendData(LOG_MESSAGE, stdioBuf, len);
}

static void ddsRelay_commit() {
    waitMs(100);
    board_ddsRelay(0, 0);
}

static void ddsRelay_vna() {
    board_ddsRelay(1, 0);
    ddsRelay_commit();
    deviceState.ddsOut = DdsOut::VNA;
}

static void ddsRelay_vfo() {
    board_ddsRelay(0, 1);
    ddsRelay_commit();
    deviceState.ddsOut = DdsOut::VFO;
}

static void ddsRelay_set(AnalyserDataSource source) {
    switch(source) {
        case AnalyserDataSource::LOG_PROBE:
        case AnalyserDataSource::LIN_PROBE:
            ddsRelay_vfo();
            break;
        case AnalyserDataSource::VNA:
            ddsRelay_vna();
            break;
    }
}

static void performAnalysis(AnalyserRequest *req) {
    analyserData.freqStart = req->freqStart;
    analyserData.freqStep = req->freqStep;
    analyserData.steps = req->numSteps;
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
            case AnalyserDataSource::LOG_PROBE:
                analyserData.data[step++] = adc_readLogarithmicProbe();
                break;
            case AnalyserDataSource::LIN_PROBE:
                analyserData.data[step++] = adc_readLinearProbe();
                break;
            case AnalyserDataSource::VNA:
                analyserData.data[step++] = adc_readVnaGainValue();
                analyserData.data[step++] = adc_readVnaPhaseValue();
        }
        freq += analyserData.freqStep;
    }

    ad985x_setFrequency(0);
}

static void cmdGetDeviceInfo(void) {
    datalink_writeFrame(DEVICE_INFO, &deviceInfo, sizeof(deviceInfo));
}

static void sendDeviceState() {
    deviceState.timeMs = currentTime;
    datalink_writeFrame(DEVICE_STATE, &deviceState, sizeof(deviceState));
}

static void cmdSetVfoFrequency(uint8_t *payload) {
    ad985x_setFrequency(*((uint32_t *) payload));
}

static void cmdAnalyserStart(uint8_t *payload) {
    if (deviceState.analyser != AnalyserState::PROCESSING) {
        AnalyserRequest *req = (AnalyserRequest *) payload;
        uint32_t freqEnd = req->freqStart + (req->freqStep * req->numSteps);
        logPrintf("start analyzer from %lu to %lu in %d steps, source: %d", req->freqStart, freqEnd, req->numSteps, req->source);
        if (req->numSteps > 0 && req->numSteps <= ANALYSER_MAX_STEPS &&
            req->freqStart >= deviceInfo.vfo.minFreq && freqEnd <= deviceInfo.vfo.maxFreq) {
            deviceState.analyser = AnalyserState::PROCESSING;
            ddsRelay_set(req->source);
            performAnalysis(req);
            sendAnalyserData();
            deviceState.analyser = AnalyserState::READY;
        } else {
            deviceState.analyser = AnalyserState::INVALID_REQUEST;
        }
    }
    waitMs(200);
    sendDeviceState();
}

static void cmdGetVfoFrequency() {
    uint32_t frequency = ad985x_frequency();
    datalink_writeFrame(VFO_GET_FREQ, &frequency, sizeof(frequency));
}

static void cmdSampleFMeter() {
    uint32_t frequency = fmeter_read();
    datalink_writeFrame(FMETER_GET, &frequency, sizeof(frequency));
}

static void cmdSampleLogarithmicProbe() {
    uint16_t value = adc_readLogarithmicProbe();
    datalink_writeFrame(LOGPROBE_GET, &value, sizeof(value));
}

static void cmdSampleLinearProbe() {
    uint16_t value = adc_readLinearProbe();
    datalink_writeFrame(LINPROBE_GET, &value, sizeof(value));
}

inline static Complex readVnaProbe() {
    return { adc_readVnaGainValue(), adc_readVnaPhaseValue() };
}

inline static Probes readAllProbes() {
    return { adc_readLogarithmicProbe(), adc_readLinearProbe(), readVnaProbe(), fmeter_read() };
}

static void cmdSampleComplexProbe() {
    Complex gp = readVnaProbe();
    datalink_writeFrame(CMPPROBE_GET, &gp, sizeof(gp));
}

static void cmdSampleProbes() {
    Probes data = readAllProbes();
    datalink_writeFrame(PROBES_GET, &data, sizeof(data));
}

static void systick_init() {
    SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);
}

extern "C" void SysTick_Handler(void) {
    static uint8_t fmeter_timebaseCounter = TICKS_PER_SECOND;

    currentTime += MS_PER_TICK;
    if (!(--fmeter_timebaseCounter)) {
        fmeter_timebase();
        fmeter_timebaseCounter = TICKS_PER_SECOND;
    }
}

void radio3_init() {
    ad985x_init();
    ad985x_setFrequency(0);
    systick_init();
    fmeter_init();
    adc_init();
    logPrintf("started!");
}

static void handleIncomingFrame() {
    static DataLinkFrame frame;
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
            sendDeviceState();
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

        case SET_DDS_RELAY_VFO:
            ddsRelay_vfo();
            break;

        case SET_DDS_RELAY_VNA:
            ddsRelay_vna();
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
    while (true) {
        handleDataSampling();
        board_ledGreen(1);
        if (datalink_isIncomingData()) {
            board_ledGreen(0);
            handleIncomingFrame();
        }
    }
}

void main() {
    board_init();
    board_ledYellow(1);

    iodev_init();
    datalink_init();
    radio3_init();
    ddsRelay_vfo();

    board_ledYellow(0);
    radio3_start();
}
