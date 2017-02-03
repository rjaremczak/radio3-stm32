/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Main application control functions
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stm32f10x.h>


#include "vfo.h"
#include "iodev.h"
#include "buildid.h"
#include "radio3.h"
#include "fmeter.h"
#include "board.h"
#include "datalink.h"
#include "adc.h"
#include "cortexm/ExceptionHandlers.h"

static const auto MAX_PAYLOAD_SIZE = 16;
static const auto TICKS_PER_SECOND = 100;
static const auto MS_PER_TICK = 10;

static const auto DEVICE_RESET = 0x000;
static const auto DEVICE_INFO = 0x001;
static const auto DEVICE_STATE = 0x002;
static const auto DEVICE_HARDWARE_REVISION = 0x003;

static const auto VFO_GET_FREQ = 0x008;
static const auto VFO_SET_FREQ = 0x009;
static const auto VFO_ERROR = 0x00f;

static const auto LOGPROBE_GET = 0x010;
static const auto LOGPROBE_INFO = 0x016;
static const auto LOGPROBE_ERROR = 0x017;

static const auto LINPROBE_GET = 0x018;
static const auto LINPROBE_ERROR = 0x01f;

static const auto CMPPROBE_GET = 0x020;
static const auto CMPPROBE_ERROR = 0x027;

static const auto FMETER_GET = 0x028;
static const auto FMETER_ERROR = 0x02f;

static const auto PROBES_GET = 0x030;
static const auto PROBES_START_SAMPLING = 0x031;
static const auto PROBES_STOP_SAMPLING = 0x032;
static const auto VFO_OUT_DIRECT = 0x033;
static const auto VFO_OUT_VNA = 0x034;
static const auto VFO_TYPE = 0x035;
static const auto VFO_ATTENUATOR = 0x036;

static const auto ANALYSER_REQUEST = 0x040;
static const auto ANALYSER_STOP = 0x041;
static const auto ANALYSER_DATA = 0x042;

static const auto ANALYSER_HEADER_SIZE = 12;
static const auto ANALYSER_MAX_STEPS = 1000;
static const auto ANALYSER_MAX_SERIES = 2;

static const auto ERROR_INVALID_FRAME = 0x3fe;
static const auto LOG_MESSAGE = 0x3ff;

static const auto STDIO_BUF_SIZE = 128;

volatile uint32_t currentTime = 0;

enum class AnalyserDataSource : uint8_t { LOG_PROBE, LIN_PROBE, VNA };

enum class AnalyserState : uint8_t { READY, PROCESSING, INVALID_REQUEST };

enum class VfoOut : uint8_t { DIRECT, VNA };

enum class HardwareRevision : uint8_t { UNKNOWN, PROTOTYPE_1, PROTOTYPE_2 };

enum class VfoAttenuator : uint8_t { LEVEL_0, LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5, LEVEL_6, LEVEL_7 };

struct Complex {
    uint16_t value;
    uint16_t phase;
} __packed;

struct DeviceState {
    uint8_t probesSampling;
    uint16_t samplingPeriodMs;
    uint32_t timeMs;
    AnalyserState analyser;
    VfoOut vfoOut;
    VfoAttenuator vfoAttenuator;
} __packed;

struct DeviceInfo {
    const char name[16];
    const char buildId[32];
    HardwareRevision hardwareRevision;
    VfoType vfoType;
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

static DeviceInfo deviceInfo = { "radio3-stm32-md", BUILD_ID, HardwareRevision::UNKNOWN, VfoType::DDS_AD9851 };
static DeviceState deviceState = { 0, 200, 0, AnalyserState::READY, VfoOut::DIRECT , VfoAttenuator::LEVEL_0 };
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

static void vfoRelayCommit() {
    waitMs(100);
    board_vfoOutBistable(0, 0);
}

static void vfoOutput_vna() {
    switch (deviceInfo.hardwareRevision) {
        case HardwareRevision::PROTOTYPE_1:
            board_vfoOutBistable(1, 0);
            vfoRelayCommit();
            break;
        case HardwareRevision::PROTOTYPE_2:
            board_vfoOut(true);
            break;
        default: return;
    }
    deviceState.vfoOut = VfoOut::VNA;
}

static void vfoOutput_direct() {
    switch (deviceInfo.hardwareRevision) {
        case HardwareRevision::PROTOTYPE_1:
            board_vfoOutBistable(0, 1);
            vfoRelayCommit();
            break;
        case HardwareRevision::PROTOTYPE_2:
            board_vfoOut(false);
            break;
        default: return;
    }
    deviceState.vfoOut = VfoOut::DIRECT;
}

static void vfoRelay_set(AnalyserDataSource source) {
    switch(source) {
        case AnalyserDataSource::LOG_PROBE:
        case AnalyserDataSource::LIN_PROBE:
            vfoOutput_direct();
            break;
        case AnalyserDataSource::VNA:
            vfoOutput_vna();
            break;
    }
}

static void performAnalysis(AnalyserRequest *req) {
    analyserData.freqStart = req->freqStart;
    analyserData.freqStep = req->freqStep;
    analyserData.steps = req->numSteps;
    analyserData.source = req->source;

    uint32_t freq = analyserData.freqStart;
    vfo_setFrequency(freq);

    // wait for initial stabilization
    volatile int w = 50000;
    while (w) { w--; }

    uint16_t numSteps = calculateAnalyserDataSteps();
    uint16_t step = 0;
    while (step <= numSteps) {
        vfo_setFrequency(freq);

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

    vfo_setFrequency(0);
}

static void cmdGetDeviceInfo(void) {
    datalink_writeFrame(DEVICE_INFO, &deviceInfo, sizeof(deviceInfo));
}

static void sendDeviceState() {
    deviceState.timeMs = currentTime;
    datalink_writeFrame(DEVICE_STATE, &deviceState, sizeof(deviceState));
}

static void cmdSetVfoFrequency(uint8_t *payload) {
    vfo_setFrequency(*((uint32_t *) payload));
}

static void cmdAnalyserStart(uint8_t *payload) {
    if (deviceState.analyser != AnalyserState::PROCESSING) {
        AnalyserRequest *req = (AnalyserRequest *) payload;
        uint32_t freqEnd = req->freqStart + (req->freqStep * req->numSteps);
        logPrintf("start analyzer from %lu to %lu in %d steps, source: %d", req->freqStart, freqEnd, req->numSteps, req->source);
        if (req->numSteps > 0 && req->numSteps <= ANALYSER_MAX_STEPS) {
            deviceState.analyser = AnalyserState::PROCESSING;
            vfoRelay_set(req->source);
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
    uint32_t frequency = vfo_frequency();
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

static void cmdVfoType(uint8_t *payload) {
    deviceInfo.vfoType = (VfoType) *payload;
    vfo_init(deviceInfo.vfoType);
    vfo_setFrequency(0);
}

static void cmdVfoAttenuator(uint8_t *payload) {
    if(deviceInfo.hardwareRevision == HardwareRevision::PROTOTYPE_2) {
        deviceState.vfoAttenuator = (VfoAttenuator) *payload;
        board_vfoAtt_1((bool) (*payload & 0b001));
        board_vfoAtt_2((bool) (*payload & 0b010));
        board_vfoAtt_3((bool) (*payload & 0b100));
    }
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

        case VFO_OUT_DIRECT:
            vfoOutput_direct();
            break;

        case VFO_OUT_VNA:
            vfoOutput_vna();
            break;

        case ANALYSER_REQUEST:
            cmdAnalyserStart(payload);
            break;

        case DEVICE_HARDWARE_REVISION:
            deviceInfo.hardwareRevision = (HardwareRevision) payload[0];
            break;

        case VFO_TYPE:
            cmdVfoType(payload);
            break;

        case VFO_ATTENUATOR:
            cmdVfoAttenuator(payload);
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
        board_ledOnModule(1);
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
    vfoOutput_direct();

    board_ledYellow(0);
    radio3_start();
}
