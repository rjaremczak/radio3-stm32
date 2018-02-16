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
#include <ElectronicSignature.h>

#include "Vfo.h"
#include "Radio3.h"
#include "cortexm/ExceptionHandlers.h"
#include "delay.h"

static Timer timer;
static Board board;
static UsbVCom usbVCom(timer);
static Radio3 radio3(usbVCom, board);

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

void Radio3::vfoToVna() {
    switch (configuration.hardwareRevision) {
        case HardwareRevision::rev1:
            board.vfoOutBistable(false, true);
            vfoRelayCommit();
            break;
        case HardwareRevision::rev2:
        case HardwareRevision::rev3:
            board.vfoOutMonostable(true);
            break;
        default:
            break;
    }
    state.vfoToVna = true;
}

void Radio3::vfoToSocket() {
    switch (configuration.hardwareRevision) {
        case HardwareRevision::rev1:
            board.vfoOutBistable(true, false);
            vfoRelayCommit();
            break;
        case HardwareRevision::rev2:
        case HardwareRevision::rev3:
            board.vfoOutMonostable(false);
            break;
        default:
            break;
    }
    state.vfoToVna = false;
}

void Radio3::adjustVfoOutputFor(Sweep::Source source) {
    switch (source) {
        case Sweep::Source::LOG_PROBE:
        case Sweep::Source::LIN_PROBE:
            vfoToSocket();
            break;
        case Sweep::Source::VNA:
            vfoToVna();
            break;
    }
}

void Radio3::sendPing() {
    sendFrame(FrameCmd::ping, nullptr, 0);
}

void Radio3::sendDeviceInfo() {
    sendFrame(FrameCmd::getDeviceConfiguration, &configuration, sizeof(configuration));
}

void Radio3::sendDeviceState() {
    state.timeMs = timer.getMillis();
    sendFrame(FrameCmd::getDeviceState, &state, sizeof(state));
}

void Radio3::cmdSetVfoFrequency(const uint8_t *payload) {
    vfo.setFrequency(*((uint32_t *) payload));
}

void Radio3::cmdGetVfoFrequency() {
    uint32_t frequency = vfo.frequency();
    sendFrame(FrameCmd::getVfoFreq, &frequency, sizeof(frequency));
}

void Radio3::cmdSweepRequest(const uint8_t *payload) {
    if (sweep.getState() != Sweep::State::PROCESSING) {
        Sweep::Request& request = *reinterpret_cast<Sweep::Request *>(const_cast<uint8_t *>(payload));
        adjustVfoOutputFor(request.source);
        sweep.perform(request);
        sendFrame(FrameCmd::sweepResponse, &sweep.getResponse(), sweep.getResponse().size());
    }
}

void Radio3::cmdVfoType(const uint8_t *payload) {
    configuration.vfoType = static_cast<Vfo::Type>(*payload);
    vfo.init(configuration.vfoType);
    vfo.setFrequency(0);
}

void Radio3::cmdVfoAttenuator(const uint8_t *payload) {
    if (configuration.hardwareRevision >= HardwareRevision::rev2) {
        state.attenuator = *payload;
        board.att1((bool) (*payload & 0b001));
        board.att2((bool) (*payload & 0b010));
        board.att3((bool) (*payload & 0b100));
    }
}

void Radio3::cmdVfoAmplifier(const uint8_t *payload) {
    if (configuration.hardwareRevision >= HardwareRevision::rev2) {
        state.amplifier = *payload;
        board.amplifier(state.amplifier);
    }
}

VnaValue Radio3::readVnaProbe() {
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

void Radio3::cmdSampleAllProbes() {
    ProbeValues values = readAllProbes();
    sendFrame(FrameCmd::getAllProbes, &values, sizeof(values));
}

void Radio3::handleIncomingFrame() {
    static DataLink::Frame frame;
    static uint8_t payload[MAX_PAYLOAD_SIZE];

    dataLink.readFrame(&frame, payload, MAX_PAYLOAD_SIZE);
    if (dataLink.error()) { return; }

    switch (static_cast<FrameCmd>(frame.command)) {
        case FrameCmd::ping:
            sendPing();
            break;

        case FrameCmd::getDeviceConfiguration:
            sendDeviceInfo();
            break;

        case FrameCmd::getDeviceState:
            sendDeviceState();
            break;

        case FrameCmd::getVfoFreq:
            cmdGetVfoFrequency();
            break;

        case FrameCmd::setVfoFreq:
            cmdSetVfoFrequency(payload);
            sendPing();
            break;

        case FrameCmd::getAllProbes:
            cmdSampleAllProbes();
            break;

        case FrameCmd::setVfoToSocket:
            vfoToSocket();
            sendPing();
            break;

        case FrameCmd::setVfoToVna:
            vfoToVna();
            sendPing();
            break;

        case FrameCmd::sweepRequest:
            cmdSweepRequest(payload);
            break;

        case FrameCmd::setVfoType:
            cmdVfoType(payload);
            sendPing();
            break;

        case FrameCmd::setAttenuator:
            cmdVfoAttenuator(payload);
            sendPing();
            break;

        case FrameCmd::setAmplifier:
            cmdVfoAmplifier(payload);
            sendPing();
            break;

        default:
            log("command %u not supported", frame.command);
    }
}

void main() {
    log_init(&timer);
    log("started");

    board.init();

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

    state.timeMs = 0;
    state.vfoToVna = false;
    state.amplifier = false;
    state.attenuator = 0;

    configuration.coreUniqueId0 = core::uniqueId.id0;
    configuration.coreUniqueId1 = core::uniqueId.id1;
    configuration.coreUniqueId2 = core::uniqueId.id2;
    configuration.firmwareVersionMajor = PROJECT_VERSION_MAJOR,
    configuration.firmwareVersionMinor = PROJECT_VERSION_MINOR,
    configuration.firmwareBuildTimestamp = BUILD_TIMESTAMP,
    configuration.hardwareRevision = board.getHardwareRevision();
    configuration.vfoType = Vfo::Type::ddsAD9851;

    fMeter.init();
    adcProbes.init();
    vfoToSocket();
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