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

#include "Vfo.h"
#include "Radio3.h"
#include "cortexm/ExceptionHandlers.h"
#include "delay.h"

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
    Vfo::Type vfoType;
    uint32_t baudRate;
} __packed;

static DeviceInfo deviceInfo = {"radio4", "", HardwareRevision::AUTODETECT, Vfo::Type::DDS_AD9851, 115200};
static DeviceState deviceState = {0, VfoOut::DIRECT, VfoAmplifier::OFF, VfoAttenuator::LEVEL_0};

static Timer timer;
static Board board;

UsbVCom usbVCom(timer);
Radio3 radio3(usbVCom, board);

extern "C" void SysTick_Handler(void) {
    radio3.sysTick();
}

void Radio3::sysTick() {
    timer.tickMs();
    fMeter.tickMs();
}

void Radio3::sendFrame(FrameCmd cmd, const void *payload, uint16_t size) {
    dataLink.writeFrame(static_cast<uint16_t>(cmd), static_cast<const uint8_t *>(payload), size);
}

void Radio3::vfoRelayCommit() {
    delayUs(100000);
    board.vfoOutBistable(false, false);
}

void Radio3::vfoOutput_vna() {
    switch (deviceInfo.hardwareRevision) {
        case HardwareRevision::VERSION_1:
            board.vfoOutBistable(false, true);
            vfoRelayCommit();
            break;
        case HardwareRevision::VERSION_2:
        case HardwareRevision::VERSION_3:
            board.vfoOutMonostable(true);
            break;
        default:
            return;
    }
    deviceState.vfoOut = VfoOut::VNA;
}

void Radio3::vfoOutput_direct() {
    switch (deviceInfo.hardwareRevision) {
        case HardwareRevision::VERSION_1:
            board.vfoOutBistable(true, false);
            vfoRelayCommit();
            break;
        case HardwareRevision::VERSION_2:
        case HardwareRevision::VERSION_3:
            board.vfoOutMonostable(false);
            break;
        default:
            return;
    }
    deviceState.vfoOut = VfoOut::DIRECT;
}

void Radio3::vfoRelay_set(Sweep::Source source) {
    switch (source) {
        case Sweep::Source::LOG_PROBE:
        case Sweep::Source::LIN_PROBE:
            vfoOutput_direct();
            break;
        case Sweep::Source::VNA:
            vfoOutput_vna();
            break;
    }
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
    vfo.setFrequency(*((uint32_t *) payload));
}

void Radio3::cmdGetVfoFrequency() {
    uint32_t frequency = vfo.frequency();
    sendFrame(FrameCmd::VFO_GET_FREQ, &frequency, sizeof(frequency));
}

void Radio3::cmdSweepRequest(uint8_t *payload) {
    if (sweep.getState() != Sweep::State::PROCESSING) {
        Sweep::Request &request = *reinterpret_cast<Sweep::Request *>(payload);
        vfoRelay_set(request.source);
        sweep.perform(request);
        sendFrame(FrameCmd::SWEEP_RESPONSE, &sweep.getResponse(), sweep.getResponse().size());
    }
}

void Radio3::cmdSampleFMeter() {
    uint32_t frequency = fMeter.read();
    sendFrame(FrameCmd::FMETER_DATA, &frequency, sizeof(frequency));
}

void Radio3::cmdSampleLogarithmicProbe() {
    uint16_t value = adcProbes.readLogarithmic(DEFAULT_AVG_SAMPLES);
    sendFrame(FrameCmd::LOGPROBE_DATA, &value, sizeof(value));
}

void Radio3::cmdSampleLinearProbe() {
    uint16_t value = adcProbes.readLinear(DEFAULT_AVG_SAMPLES);
    sendFrame(FrameCmd::LINPROBE_DATA, &value, sizeof(value));
}

void Radio3::cmdVfoType(const uint8_t *payload) {
    deviceInfo.vfoType = (Vfo::Type) *payload;
    vfo.init(deviceInfo.vfoType);
    vfo.setFrequency(0);
}

void Radio3::cmdVfoAttenuator(const uint8_t *payload) {
    if (deviceInfo.hardwareRevision >= HardwareRevision::VERSION_2) {
        deviceState.vfoAttenuator = (VfoAttenuator) *payload;
        board.att1((bool) (*payload & 0b001));
        board.att2((bool) (*payload & 0b010));
        board.att3((bool) (*payload & 0b100));
    }
}

void Radio3::cmdVfoAmplifier(const uint8_t *payload) {
    if (deviceInfo.hardwareRevision >= HardwareRevision::VERSION_2) {
        deviceState.vfoAmplifier = (VfoAmplifier) *payload;
        board.amplifier((bool) *payload);
    }
}

Complex Radio3::readVnaProbe() {
    return { adcProbes.readVnaGain(DEFAULT_AVG_SAMPLES), adcProbes.readVnaPhase(DEFAULT_AVG_SAMPLES) };
}

ProbeValues Radio3::readAllProbes() {
    return {
            adcProbes.readLogarithmic(DEFAULT_AVG_SAMPLES),
            adcProbes.readLinear(DEFAULT_AVG_SAMPLES),
            readVnaProbe(),
            fMeter.read()
    };
}

void Radio3::cmdSampleComplexProbe() {
    Complex gp = readVnaProbe();
    sendFrame(FrameCmd::CMPPROBE_DATA, &gp, sizeof(gp));
}

void Radio3::cmdSampleAllProbes() {
    ProbeValues values = readAllProbes();
    sendFrame(FrameCmd::PROBES_DATA, &values, sizeof(values));
}

void Radio3::cmdHardwareRevision(HardwareRevision hardwareRevision) {
    if (hardwareRevision == HardwareRevision::AUTODETECT) {
        deviceInfo.hardwareRevision = board.detectHardwareRevision();
    } else {
        deviceInfo.hardwareRevision = hardwareRevision;
    }

    board.init();
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
            cmdSweepRequest(payload);
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

void main() {
    snprintf(const_cast<char *>(deviceInfo.buildId), sizeof(deviceInfo.buildId), "%d.%d-%d-%06d", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, BUILD_DATE, BUILD_TIME);

    log_init(&timer);
    log("%s started (build: %s)", deviceInfo.name, deviceInfo.buildId);

    board.preInit();
    usbVCom.init();
    radio3.init();
    radio3.start();
}

Radio3::Radio3(UsbVCom &usbVCom, Board &board) :
        usbVCom(usbVCom),
        board(board),
        dataLink(usbVCom),
        sweep(vfo, adcProbes)
{}

void Radio3::init() {
    SysTick_Config(SystemCoreClock / 1000);
    fMeter.init();
    adcProbes.init();
    vfoOutput_direct();
}

void Radio3::start() {
    sweep.init();

    mainLoop:

    board.indicator(true);
    if (dataLink.isIncomingData()) {
        board.indicator(false);
        handleIncomingFrame();
    }

    goto mainLoop;
}