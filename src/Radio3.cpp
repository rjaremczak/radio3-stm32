/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Main application control functions
 *
 */

#include <cstdio>

#include <stm32f10x.h>
#include <Timer.h>
#include <UsbVCom.h>
#include <log.h>

#include "vfo.h"
#include "buildid.h"
#include "Radio3.h"
#include "fMeter.h"
#include "board.h"
#include "DataLink.h"
#include "adc.h"
#include "cortexm/ExceptionHandlers.h"
#include "delay.h"

enum class SweepState : uint8_t {
    READY, PROCESSING, INVALID_REQUEST
};

enum class VfoOut : uint8_t {
    DIRECT, VNA
};

enum class VfoAttenuator : uint8_t {
    LEVEL_0, LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5, LEVEL_6, LEVEL_7
};

enum class VfoAmplifier : uint8_t {
    OFF, ON
};

enum class LogLevel : uint8_t {
    DEBUG, INFO, ERROR
};

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

struct SweepResponse {
    SweepState state;
    uint32_t freqStart;
    uint32_t freqStep;
    uint16_t steps;
    SweepSignalSource source;
    uint16_t data[SWEEP_MAX_SERIES * (SWEEP_MAX_STEPS + 1)];

    uint16_t totalSamples() {
        const auto ns = (uint16_t) (steps + 1);
        return (uint16_t) (source == SweepSignalSource::VNA ? ns * 2 : ns);
    }

    uint16_t size() {
        return SWEEP_HEADER_SIZE + totalSamples() * sizeof(uint16_t);
    }

} __packed;

static DeviceInfo deviceInfo = {"radio3-stm32-md", BUILD_ID, HardwareRevision::AUTODETECT, VfoType::DDS_AD9851, 115200};
static DeviceState deviceState = {0, VfoOut::DIRECT, VfoAmplifier::OFF, VfoAttenuator::LEVEL_0};
static SweepResponse sweepResponse;

static Timer timer;
UsbVCom usbVCom(timer);
Radio3 radio3(usbVCom);

extern "C" void SysTick_Handler(void) {
    radio3.sysTick();
}

void Radio3::sysTick() {
    timer.tickMs();
    fMeter.tickMs();
}

void Radio3::sendFrame(FrameCmd cmd, void *payload, uint16_t size) {
    dataLink.writeFrame(static_cast<uint16_t>(cmd), static_cast<uint8_t *>(payload), size);
}

void Radio3::sendSweepResponse() {
    sendFrame(FrameCmd::SWEEP_RESPONSE, (uint8_t *) &sweepResponse, sweepResponse.size());
}

void Radio3::vfoRelayCommit() {
    delayUs(100000);
    board_vfoOutBistable(false, false);
}

void Radio3::vfoOutput_vna() {
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

void Radio3::vfoOutput_direct() {
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

void Radio3::vfoRelay_set(SweepSignalSource source) {
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

void Radio3::resetSweepData(const uint16_t totalSamples) {
    for (uint16_t i = 0; i < totalSamples; i++) { sweepResponse.data[i] = 0; }
}

void Radio3::divideAccumulatedData(const uint16_t totalSamples, const uint8_t divider) {
    for (uint16_t step = 0; step < totalSamples; step++) { sweepResponse.data[step] /= divider; }
}

void Radio3::sweepAndAccumulate(const uint16_t totalSamples, const uint8_t avgSamples) {
    uint32_t freq = sweepResponse.freqStart;
    uint16_t step = 0;

    vfo_setFrequency(freq);
    delayUs(3);
    adc_readLogarithmicProbe(avgSamples);

    while (step < totalSamples) {
        vfo_setFrequency(freq);
        delayUs(3);
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

void Radio3::performSweep(SweepRequest *req) {
    sweepResponse.freqStart = req->freqStart;
    sweepResponse.freqStep = req->freqStep;
    sweepResponse.steps = req->steps;
    sweepResponse.source = req->source;

    const auto avgPasses = req->getAvgPasses();
    const auto avgSamples = req->getAvgSamples();
    const auto totalSamples = sweepResponse.totalSamples();

    auto prevFreq = vfo_frequency();
    resetSweepData(totalSamples);
    for (uint8_t i = 0; i < avgPasses; i++) { sweepAndAccumulate(totalSamples, avgSamples); }
    divideAccumulatedData(totalSamples, avgPasses);
    vfo_setFrequency(prevFreq);
}

void Radio3::sendPing() {
    sendFrame(FrameCmd::PING, nullptr, 0);
}

void Radio3::sendDeviceInfo() {
    deviceInfo.baudRate = 0;
    sendFrame(FrameCmd::DEVICE_INFO, &deviceInfo, sizeof(deviceInfo));
}

void Radio3::sendDeviceState() {
    deviceState.timeMs = timer.getMillis();
    sendFrame(FrameCmd::DEVICE_STATE, &deviceState, sizeof(deviceState));
}

void Radio3::cmdSetVfoFrequency(const uint8_t *payload) {
    vfo_setFrequency(*((uint32_t *) payload));
}

void Radio3::cmdGetVfoFrequency() {
    uint32_t frequency = vfo_frequency();
    sendFrame(FrameCmd::VFO_GET_FREQ, &frequency, sizeof(frequency));
}

void Radio3::cmdSweepStart(uint8_t *payload) {
    if (sweepResponse.state != SweepState::PROCESSING) {
        auto *req = reinterpret_cast<SweepRequest *>(payload);
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

void Radio3::cmdSampleFMeter() {
    uint32_t frequency = fMeter.read();
    sendFrame(FrameCmd::FMETER_DATA, &frequency, sizeof(frequency));
}

void Radio3::cmdSampleLogarithmicProbe() {
    uint16_t value = adc_readLogarithmicProbe(DEFAULT_AVG_SAMPLES);
    sendFrame(FrameCmd::LOGPROBE_DATA, &value, sizeof(value));
}

void Radio3::cmdSampleLinearProbe() {
    uint16_t value = adc_readLinearProbe(DEFAULT_AVG_SAMPLES);
    sendFrame(FrameCmd::LINPROBE_DATA, &value, sizeof(value));
}

void Radio3::cmdVfoType(const uint8_t *payload) {
    deviceInfo.vfoType = (VfoType) *payload;
    vfo_init(deviceInfo.vfoType);
    vfo_setFrequency(0);
}

void Radio3::cmdVfoAttenuator(const uint8_t *payload) {
    if (deviceInfo.hardwareRevision == HardwareRevision::VERSION_2) {
        deviceState.vfoAttenuator = (VfoAttenuator) *payload;
        board_vfoAtt1((bool) (*payload & 0b001));
        board_vfoAtt2((bool) (*payload & 0b010));
        board_vfoAtt3((bool) (*payload & 0b100));
    }
}

void Radio3::cmdVfoAmplifier(const uint8_t *payload) {
    if (deviceInfo.hardwareRevision == HardwareRevision::VERSION_2) {
        deviceState.vfoAmplifier = (VfoAmplifier) *payload;
        board_vfoAmplifier((bool) *payload);
    }
}

Complex Radio3::readVnaProbe() {
    return {adc_readVnaGainValue(DEFAULT_AVG_SAMPLES), adc_readVnaPhaseValue(DEFAULT_AVG_SAMPLES)};
}

Probes Radio3::readAllProbes() {
    return {
            adc_readLogarithmicProbe(DEFAULT_AVG_SAMPLES),
            adc_readLinearProbe(DEFAULT_AVG_SAMPLES),
            readVnaProbe(),
            fMeter.read()
    };
}

void Radio3::cmdSampleComplexProbe() {
    Complex gp = readVnaProbe();
    sendFrame(FrameCmd::CMPPROBE_DATA, &gp, sizeof(gp));
}

void Radio3::cmdSampleAllProbes() {
    Probes data = readAllProbes();
    sendFrame(FrameCmd::PROBES_DATA, &data, sizeof(data));
}

void Radio3::cmdHardwareRevision(HardwareRevision hardwareRevision) {
    if (hardwareRevision == HardwareRevision::AUTODETECT) {
        deviceInfo.hardwareRevision = board_isRevision2() ? HardwareRevision::VERSION_2 : HardwareRevision::VERSION_1;
    } else {
        deviceInfo.hardwareRevision = hardwareRevision;
    }

    board_init();
}

void Radio3::handleIncomingFrame() {
    static DataLink::Frame frame;
    static uint8_t payload[MAX_PAYLOAD_SIZE];

    dataLink.readFrame(&frame, payload, MAX_PAYLOAD_SIZE);
    if (dataLink.error()) { return; }

    switch (static_cast<FrameCmd>(frame.command)) {
        case FrameCmd::PING:
            sendPing();
            break;

        case FrameCmd::DEVICE_INFO:
            sendDeviceInfo();
            break;

        case FrameCmd::DEVICE_STATE:
            sendDeviceState();
            break;

        case FrameCmd::VFO_GET_FREQ:
            cmdGetVfoFrequency();
            break;

        case FrameCmd::VFO_SET_FREQ:
            cmdSetVfoFrequency(payload);
            sendPing();
            break;

        case FrameCmd::LOGPROBE_DATA:
            cmdSampleLogarithmicProbe();
            break;

        case FrameCmd::LINPROBE_DATA:
            cmdSampleLinearProbe();
            break;

        case FrameCmd::CMPPROBE_DATA:
            cmdSampleComplexProbe();
            break;

        case FrameCmd::FMETER_DATA:
            cmdSampleFMeter();
            break;

        case FrameCmd::PROBES_DATA:
            cmdSampleAllProbes();
            break;

        case FrameCmd::VFO_OUT_DIRECT:
            vfoOutput_direct();
            sendPing();
            break;

        case FrameCmd::VFO_OUT_VNA:
            vfoOutput_vna();
            sendPing();
            break;

        case FrameCmd::SWEEP_REQUEST:
            cmdSweepStart(payload);
            break;

        case FrameCmd::DEVICE_HARDWARE_REVISION:
            cmdHardwareRevision((HardwareRevision) payload[0]);
            sendPing();
            break;

        case FrameCmd::VFO_TYPE:
            cmdVfoType(payload);
            sendPing();
            break;

        case FrameCmd::VFO_ATTENUATOR:
            cmdVfoAttenuator(payload);
            sendPing();
            break;

        case FrameCmd::VFO_AMPLIFIER:
            cmdVfoAmplifier(payload);
            sendPing();
            break;

        default:
            log("command %u not supported", frame.command);
    }
}

int main() {
    log_init(&timer);
    log("radio3 started");

    board_preInit();
    usbVCom.init();
    radio3.init();
    radio3.start();

    return 0;
}

Radio3::Radio3(ComDevice &comDevice) : comDevice(comDevice), dataLink(comDevice) {}

void Radio3::init() {
    SysTick_Config(SystemCoreClock / 1000);
    fMeter.init();
    adc_init();
    vfoOutput_direct();
}

void Radio3::start() {
    sweepResponse.state = SweepState::READY;

    while (true) {
        board_indicator(true);
        if (dataLink.isIncomingData()) {
            board_indicator(false);
            handleIncomingFrame();
        }
    }
}