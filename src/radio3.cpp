/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Main application control functions
 *
 */

#include <stdio.h>
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
#include "delay.h"

static const auto DEFAULT_AVG_SAMPLES = 3;

static const auto MAX_PAYLOAD_SIZE = 16;
static const auto TICKS_PER_SECOND = 100;
static const auto MS_PER_TICK = 10;

static const auto PING = 0x000;
static const auto DEVICE_INFO = 0x001;
static const auto DEVICE_STATE = 0x002;
static const auto DEVICE_HARDWARE_REVISION = 0x003;

static const auto VFO_GET_FREQ = 0x008;
static const auto VFO_SET_FREQ = 0x009;

static const auto LOGPROBE_DATA = 0x010;

static const auto LINPROBE_DATA = 0x018;

static const auto CMPPROBE_DATA = 0x020;

static const auto FMETER_DATA = 0x028;

static const auto PROBES_DATA = 0x030;
static const auto VFO_OUT_DIRECT = 0x033;
static const auto VFO_OUT_VNA = 0x034;
static const auto VFO_TYPE = 0x035;
static const auto VFO_ATTENUATOR = 0x036;
static const auto VFO_AMPLIFIER = 0x037;
static const auto VNA_MODE = 0x038;

static const auto SWEEP_REQUEST = 0x040;
static const auto SWEEP_RESPONSE = 0x041;

static const auto SWEEP_HEADER_SIZE = 12;
static const auto SWEEP_MAX_STEPS = 1000;
static const auto SWEEP_MAX_SERIES = 2;

volatile uint32_t currentTime = 0;

enum class SweepSignalSource : uint8_t {
    LOG_PROBE, LIN_PROBE, VNA
};

enum class SweepState : uint8_t {
    READY, PROCESSING, INVALID_REQUEST
};

enum class VfoOut : uint8_t {
    DIRECT, VNA
};

enum class HardwareRevision : uint8_t {
    AUTODETECT, VERSION_1, VERSION_2
};

enum class VfoAttenuator : uint8_t {
    LEVEL_0, LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5, LEVEL_6, LEVEL_7
};

enum class VfoAmplifier : uint8_t {
    OFF, ON
};

enum class VnaMode : uint8_t {
    DIRECTIONAL_COUPLER, BRIDGE
};

enum class LogLevel : uint8_t {
    DEBUG, INFO, ERROR
};

struct Complex {
    uint16_t value;
    uint16_t phase;
} __packed;

struct DeviceState {
    uint32_t timeMs;
    VfoOut vfoOut;
    VfoAmplifier vfoAmplifier;
    VfoAttenuator vfoAttenuator;
} __packed;

struct DeviceInfo {
    const char name[16];
    const char buildId[32];
    HardwareRevision hardwareRevision;
    VfoType vfoType;
    uint32_t baudRate;
} __packed;

struct Probes {
    uint16_t logarithmic;
    uint16_t linear;
    Complex complex;
    uint32_t fMeter;
} __packed;

struct SweepRequest {
    uint32_t freqStart;
    uint32_t freqStep;
    uint16_t steps;
    SweepSignalSource source;
    uint8_t avgMode;

    bool isValid() {
        return steps > 0 && steps <= SWEEP_MAX_STEPS && freqStep > 0;
    }

    uint8_t getAvgSamples() {
        return (uint8_t) ((avgMode & 0x0f) + 1);
    }

    uint8_t getAvgPasses() {
        return (uint8_t) ((avgMode >> 4 & 0x0f) + 1);
    }

} __packed;

struct SweepResponse {
    SweepState state;
    uint32_t freqStart;
    uint32_t freqStep;
    uint16_t steps;
    SweepSignalSource source;
    uint16_t data[SWEEP_MAX_SERIES * (SWEEP_MAX_STEPS + 1)];

    uint16_t totalSamples() {
        const uint16_t ns = (uint16_t) (steps + 1);
        return (uint16_t) (source == SweepSignalSource::VNA ? ns * 2 : ns);
    }

    uint16_t size() {
        return SWEEP_HEADER_SIZE + totalSamples() * sizeof(uint16_t);
    }

} __packed;

static DeviceInfo deviceInfo = { "radio3-stm32-md", BUILD_ID, HardwareRevision::AUTODETECT, VfoType::DDS_AD9851, 115200 };
static DeviceState deviceState = {0, VfoOut::DIRECT, VfoAmplifier::OFF, VfoAttenuator::LEVEL_0 };
static SweepResponse sweepResponse;

static void sendData(uint16_t command, void *payload, uint16_t size) {
    datalink_writeFrame(command, payload, size);
}

static void sendSweepResponse() {
    sendData(SWEEP_RESPONSE, &sweepResponse, sweepResponse.size());
}

static void vfoRelayCommit() {
    delayUs(100000);
    board_vfoOutBistable(false, false);
}

static void vfoOutput_vna() {
    switch (deviceInfo.hardwareRevision) {
        case HardwareRevision::VERSION_1:
            board_vfoOutBistable(false, true);
            vfoRelayCommit();
            break;
        case HardwareRevision::VERSION_2:
            board_vfoOut(true);
            break;
        default:
            return;
    }
    deviceState.vfoOut = VfoOut::VNA;
}

static void vfoOutput_direct() {
    switch (deviceInfo.hardwareRevision) {
        case HardwareRevision::VERSION_1:
            board_vfoOutBistable(true, false);
            vfoRelayCommit();
            break;
        case HardwareRevision::VERSION_2:
            board_vfoOut(false);
            break;
        default:
            return;
    }
    deviceState.vfoOut = VfoOut::DIRECT;
}

static void vfoRelay_set(SweepSignalSource source) {
    switch (source) {
        case SweepSignalSource::LOG_PROBE:
        case SweepSignalSource::LIN_PROBE:
            vfoOutput_direct();
            break;
        case SweepSignalSource::VNA:
            vfoOutput_vna();
            break;
    }
}

static void resetSweepData(const uint16_t totalSamples) {
    for (uint16_t i = 0; i < totalSamples; i++) { sweepResponse.data[i] = 0; }
}

static void divideAccumulatedData(const uint16_t totalSamples, const uint8_t divider) {
    for (uint16_t step = 0; step < totalSamples; step++) { sweepResponse.data[step] /= divider; }
}

static void sweepAndAccumulate(const uint16_t totalSamples, const uint8_t avgSamples) {
    uint32_t freq = sweepResponse.freqStart;
    uint16_t step = 0;
    vfo_setFrequency(freq);
    delayUs(100000);

    while (step < totalSamples) {
        vfo_setFrequency(freq);
        delayUs(5);
        switch (sweepResponse.source) {
            case SweepSignalSource::LOG_PROBE:
                sweepResponse.data[step++] += adc_readLogarithmicProbe(avgSamples);
                break;
            case SweepSignalSource::LIN_PROBE:
                sweepResponse.data[step++] += adc_readLinearProbe(avgSamples);
                break;
            case SweepSignalSource::VNA:
                sweepResponse.data[step++] += adc_readVnaGainValue(avgSamples);
                sweepResponse.data[step++] += adc_readVnaPhaseValue(avgSamples);
        }
        freq += sweepResponse.freqStep;
    }
}

static void performSweep(SweepRequest *req) {
    sweepResponse.freqStart = req->freqStart;
    sweepResponse.freqStep = req->freqStep;
    sweepResponse.steps = req->steps;
    sweepResponse.source = req->source;

    const auto avgPasses = req->getAvgPasses();
    const auto avgSamples = req->getAvgSamples();
    const auto totalSamples = sweepResponse.totalSamples();

    resetSweepData(totalSamples);
    for (uint8_t i = 0; i < avgPasses; i++) { sweepAndAccumulate(totalSamples, avgSamples); }
    divideAccumulatedData(totalSamples, avgPasses);
    vfo_setFrequency(0);
}

static void sendPing() {
    datalink_writeFrame(PING, nullptr, 0);
}

static void sendDeviceInfo() {
    deviceInfo.baudRate = iodev_baudRate();
    datalink_writeFrame(DEVICE_INFO, &deviceInfo, sizeof(deviceInfo));
}

static void sendDeviceState() {
    deviceState.timeMs = currentTime;
    datalink_writeFrame(DEVICE_STATE, &deviceState, sizeof(deviceState));
}

static void cmdSetVfoFrequency(uint8_t *payload) {
    vfo_setFrequency(*((uint32_t *) payload));
}

static void cmdGetVfoFrequency() {
    uint32_t frequency = vfo_frequency();
    datalink_writeFrame(VFO_GET_FREQ, &frequency, sizeof(frequency));
}

static void cmdSweepStart(uint8_t *payload) {
    if (sweepResponse.state != SweepState::PROCESSING) {
        SweepRequest *req = (SweepRequest *) payload;
        if (req->isValid() && req->steps <= SWEEP_MAX_STEPS) {
            sweepResponse.state = SweepState::PROCESSING;
            vfoRelay_set(req->source);
            performSweep(req);
            sweepResponse.state = SweepState::READY;
        } else {
            sweepResponse.steps = 0;
            sweepResponse.state = SweepState::INVALID_REQUEST;
        }

        sendSweepResponse();
    }
}

static void cmdSampleFMeter() {
    uint32_t frequency = fmeter_read();
    datalink_writeFrame(FMETER_DATA, &frequency, sizeof(frequency));
}

static void cmdSampleLogarithmicProbe() {
    uint16_t value = adc_readLogarithmicProbe(DEFAULT_AVG_SAMPLES);
    datalink_writeFrame(LOGPROBE_DATA, &value, sizeof(value));
}

static void cmdSampleLinearProbe() {
    uint16_t value = adc_readLinearProbe(DEFAULT_AVG_SAMPLES);
    datalink_writeFrame(LINPROBE_DATA, &value, sizeof(value));
}

static void cmdVfoType(uint8_t *payload) {
    deviceInfo.vfoType = (VfoType) *payload;
    vfo_init(deviceInfo.vfoType);
    vfo_setFrequency(0);
}

static void cmdVfoAttenuator(uint8_t *payload) {
    if (deviceInfo.hardwareRevision == HardwareRevision::VERSION_2) {
        deviceState.vfoAttenuator = (VfoAttenuator) *payload;
        board_vfoAtt1((bool) (*payload & 0b001));
        board_vfoAtt2((bool) (*payload & 0b010));
        board_vfoAtt3((bool) (*payload & 0b100));
    }
}

static void cmdVfoAmplifier(uint8_t *payload) {
    if (deviceInfo.hardwareRevision == HardwareRevision::VERSION_2) {
        deviceState.vfoAmplifier = (VfoAmplifier) *payload;
        board_vfoAmplifier((bool) *payload);
    }
}

static void cmdVnaMode(uint8_t *payload) {
    VnaMode vnaMode = (VnaMode) *payload;
    board_vnaMode(vnaMode == VnaMode::BRIDGE);
}

inline static Complex readVnaProbe() {
    return {adc_readVnaGainValue(DEFAULT_AVG_SAMPLES), adc_readVnaPhaseValue(DEFAULT_AVG_SAMPLES)};
}

inline static Probes readAllProbes() {
    return {adc_readLogarithmicProbe(DEFAULT_AVG_SAMPLES), adc_readLinearProbe(DEFAULT_AVG_SAMPLES), readVnaProbe(),
            fmeter_read()};
}

static void cmdSampleComplexProbe() {
    Complex gp = readVnaProbe();
    datalink_writeFrame(CMPPROBE_DATA, &gp, sizeof(gp));
}

static void cmdSampleAllProbes() {
    Probes data = readAllProbes();
    datalink_writeFrame(PROBES_DATA, &data, sizeof(data));
}

static void cmdHardwareRevision(HardwareRevision hardwareRevision) {
    if (hardwareRevision == HardwareRevision::AUTODETECT) {
        deviceInfo.hardwareRevision = board_isRevision2() ? HardwareRevision::VERSION_2 : HardwareRevision::VERSION_1;
    } else {
        deviceInfo.hardwareRevision = hardwareRevision;
    }

    board_init();
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
}

static void handleIncomingFrame() {
    static DataLinkFrame frame;
    static uint8_t payload[MAX_PAYLOAD_SIZE];

    datalink_readFrame(&frame, payload, MAX_PAYLOAD_SIZE);
    if (datalink_error()) { return; }

    switch (frame.command) {
        case PING:
            sendPing();
            break;

        case DEVICE_INFO:
            sendDeviceInfo();
            break;

        case DEVICE_STATE:
            sendDeviceState();
            break;

        case VFO_GET_FREQ:
            cmdGetVfoFrequency();
            break;

        case VFO_SET_FREQ:
            cmdSetVfoFrequency(payload);
            sendPing();
            break;

        case LOGPROBE_DATA:
            cmdSampleLogarithmicProbe();
            break;

        case LINPROBE_DATA:
            cmdSampleLinearProbe();
            break;

        case CMPPROBE_DATA:
            cmdSampleComplexProbe();
            break;

        case FMETER_DATA:
            cmdSampleFMeter();
            break;

        case PROBES_DATA:
            cmdSampleAllProbes();
            break;

        case VFO_OUT_DIRECT:
            vfoOutput_direct();
            sendPing();
            break;

        case VFO_OUT_VNA:
            vfoOutput_vna();
            sendPing();
            break;

        case SWEEP_REQUEST:
            cmdSweepStart(payload);
            break;

        case DEVICE_HARDWARE_REVISION:
            cmdHardwareRevision((HardwareRevision) payload[0]);
            sendPing();
            break;

        case VFO_TYPE:
            cmdVfoType(payload);
            sendPing();
            break;

        case VFO_ATTENUATOR:
            cmdVfoAttenuator(payload);
            sendPing();
            break;

        case VFO_AMPLIFIER:
            cmdVfoAmplifier(payload);
            sendPing();
            break;

        case VNA_MODE:
            cmdVnaMode(payload);
            sendPing();
            break;
    }
}

void radio3_start() {
    sweepResponse.state = SweepState::READY;

    while (true) {
        board_indicator(true);
        if (datalink_isIncomingData()) {
            board_indicator(false);
            handleIncomingFrame();
        }
    }
}

void main() {
    board_preInit();
    iodev_init();
    datalink_init();
    radio3_init();
    vfoOutput_direct();
    radio3_start();
}
