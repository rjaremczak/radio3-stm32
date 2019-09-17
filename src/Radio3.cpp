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
#include "license.h"
#include "Vfo.h"
#include "Radio3.h"
#include "delay.h"

// SQ6DGT
//static const uint8_t encryptedLicenseData[]{0x35,0xF2,0x37,0x67,0x39,0x85,0x56,0x60,0x94,0xE2,0x7B,0x60,0xF9,0xE7,0xE2,0x9F,0xE3,0xA1,0x16,0x9A,0x9F,0xD2,0x28,0x88,0x1F,0x02,0x68,0x05,0x2C,0x5B,0x13,0x4C,0x2F,0xAA,0x40,0xE4,0xC0,0x0B,0x27,0xCE,0xB8,0x6F,0x1D,0xF9,0x38,0xA8,0x3D,0x20,0xCB,0xDF,0x01,0x45,0x95,0xDF,0x50,0x92,0x7E,0x8E,0x79,0x6D};

// SP6IFN
//static const uint8_t encryptedLicenseData[]{0x32,0xF2,0x33,0x67,0x39,0x85,0x56,0x60,0x94,0xE2,0x7B,0x60,0xF9,0xE7,0xE2,0x9F,0xE3,0xA1,0x16,0x9A,0x9F,0xD2,0x28,0x88,0x1F,0x02,0x68,0x05,0x2F,0xB4,0x0B,0x4A,0x2F,0xBC,0x51,0xFB,0xD3,0x0D,0x63,0xA4,0x8A,0x4D,0x4E,0xDD,0x1D,0x9C,0x5C,0x4B,0xEB,0x8C,0x50,0x73,0xD1,0x98,0x04,0x92,0x0F,0x9F,0x68,0x6D};

// SP6NIN
//static const uint8_t encryptedLicenseData[]{0x35,0xF2,0x30,0x67,0x39,0x85,0x56,0x60,0x94,0xE2,0x7B,0x60,0xF9,0xE7,0xE2,0x9F,0xE3,0xA1,0x16,0x9A,0x9F,0xD2,0x28,0x88,0x1F,0x02,0x68,0x05,0x2B,0xB5,0x10,0x4B,0x30,0xA4,0x50,0xE4,0xD9,0x5F,0x54,0xD4,0xEF,0x53,0x31,0xDA,0x5B,0xD2,0x5C,0x4B,0xEB,0x8C,0x50,0x73,0xD1,0x98,0x04,0x92,0x7D,0xF8,0x5A,0x8B};

// SP6EFY
//static const uint8_t encryptedLicenseData[]{0x35,0xF2,0x32,0x67,0x39,0x85,0x56,0x60,0x94,0xBD,0x3A,0x60,0xA0,0xEF,0xF2,0xE7,0xF5,0x88,0x38,0xA0,0xFB,0xA2,0x49,0xFA,0x6B,0x02,0x68,0x05,0x17,0x5B,0x12,0x49,0x37,0xA4,0x4C,0xF4,0xC1,0x05,0x27,0xD7,0x89,0x2B,0x3D,0xD2,0x02,0xD2,0x5C,0x4B,0xEB,0x8C,0x50,0x73,0xD1,0x98,0x04,0x92,0x7D,0x8D,0x6E,0x8D};

// SP6ECA
//static const uint8_t encryptedLicenseData[]{0x2D,0xF2,0x31,0x67,0x39,0x85,0x56,0x60,0x94,0xE2,0x7B,0x60,0xF9,0xE7,0xE2,0x9F,0xE3,0xA1,0x16,0x9A,0x9F,0xD2,0x28,0x88,0x1F,0x02,0x68,0x05,0x17,0x5A,0x10,0x54,0x3C,0xAB,0x46,0xF3,0xC8,0x1A,0x6D,0xA4,0x8A,0x4D,0x4E,0xD1,0x18,0x93,0x5C,0x4B,0xEB,0x8C,0x50,0x73,0xD1,0x98,0x04,0x92,0x4F,0x99,0x6D,0x8D};

static const uint8_t encryptedLicenseData[]{0x35,0xF2,0x32,0x67,0x39,0x85,0x56,0x60,0x94,0xBD,0x3A,0x60,0xA0,0xEF,0xF2,0xE7,0xF5,0x88,0x38,0xA0,0xFB,0xA2,0x49,0xFA,0x6B,0x02,0x68,0x05,0x17,0x5B,0x12,0x49,0x37,0xA4,0x4C,0xF4,0xC1,0x05,0x27,0xD7,0x89,0x2B,0x3D,0xD2,0x02,0xD2,0x5C,0x4B,0xEB,0x8C,0x50,0x73,0xD1,0x98,0x04,0x92,0x7D,0x8D,0x6E,0x8D};

static bool checkLicense() {
    LicenseData licenseData;
    decryptLicenseData(encryptedLicenseData, reinterpret_cast<uint8_t *>(&licenseData));
    return licenseData.uniqueId0 == core::uniqueId0() && licenseData.uniqueId1 == core::uniqueId1() && licenseData.uniqueId2 == core::uniqueId2();
}

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

void Radio3::sendDeviceConfiguration() {
    sendFrame(FrameCmd::getDeviceConfiguration, &configuration, sizeof(configuration));
}

void Radio3::sendDeviceState() {
    state.timeMs = timer.getMillis();
    sendFrame(FrameCmd::getDeviceState, &state, sizeof(state));
}

void Radio3::sendLicenseData() {
    uint8_t buf[sizeof(LicenseData)];
    decryptLicenseData(encryptedLicenseData, buf);
    sendFrame(FrameCmd::getLicenseData, buf, sizeof(LicenseData));
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
            sendDeviceConfiguration();
            break;

        case FrameCmd::getDeviceState:
            sendDeviceState();
            break;

        case FrameCmd::getLicenseData:
            sendLicenseData();
            break;

        case FrameCmd::getVfoFreq:
            if(configuration.licenseOk) cmdGetVfoFrequency();
            break;

        case FrameCmd::setVfoFreq:
            if(configuration.licenseOk) cmdSetVfoFrequency(payload);
            sendPing();
            break;

        case FrameCmd::getAllProbes:
            if(configuration.licenseOk) cmdSampleAllProbes();
            break;

        case FrameCmd::setVfoToSocket:
            if(configuration.licenseOk) vfoToSocket();
            sendPing();
            break;

        case FrameCmd::setVfoToVna:
            if(configuration.licenseOk) vfoToVna();
            sendPing();
            break;

        case FrameCmd::setVfoType:
            if(configuration.licenseOk) cmdVfoType(payload);
            sendPing();
            break;

        case FrameCmd::setAttenuator:
            if(configuration.licenseOk) cmdVfoAttenuator(payload);
            sendPing();
            break;

        case FrameCmd::setAmplifier:
            if(configuration.licenseOk) cmdVfoAmplifier(payload);
            sendPing();
            break;

        case FrameCmd::sweepRequest:
            if(configuration.licenseOk) cmdSweepRequest(payload);
            break;

        default:
            sendPing();
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

    configuration.licenseOk = checkLicense();
    configuration.coreUniqueId0 = core::uniqueId0();
    configuration.coreUniqueId1 = core::uniqueId1();
    configuration.coreUniqueId2 = core::uniqueId2();
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
