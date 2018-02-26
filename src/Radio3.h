/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include "DataLink.h"
#include "FreqMeter.h"
#include "AdcProbes.h"
#include "Ad985x.h"
#include "Sweep.h"
#include "Board.h"

namespace {
    const auto DEFAULT_AVG_SAMPLES = 3;
    const auto MAX_PAYLOAD_SIZE = 16;
}

enum class FrameCmd : uint16_t {
    ping = 0x000,
    getDeviceConfiguration = 0x001,
    getDeviceState = 0x002,
    getLicenseData = 0x003,

    getVfoFreq = 0x010,
    setVfoFreq = 0x011,
    getAllProbes = 0x012,
    setVfoToSocket = 0x013,
    setVfoToVna = 0x014,
    setVfoType = 0x015,
    setAttenuator = 0x016,
    setAmplifier = 0x017,

    sweepRequest = 0x030,
    sweepResponse = 0x031
};

struct VnaValue {
    uint16_t value;
    uint16_t phase;
} __attribute__((packed));

struct ProbeValues {
    uint16_t logarithmic;
    uint16_t linear;
    VnaValue complex;
    uint32_t fMeter;
} __attribute__((packed));

struct State {
    uint32_t timeMs;
    bool vfoToVna;
    bool amplifier;
    uint8_t attenuator;
}  __attribute__((packed));

struct Configuration {
    bool licenseOk = false;
    uint32_t coreUniqueId0;
    uint32_t coreUniqueId1;
    uint32_t coreUniqueId2;
    uint8_t firmwareVersionMajor;
    uint8_t firmwareVersionMinor;
    uint32_t firmwareBuildTimestamp;
    HardwareRevision hardwareRevision;
    Vfo::Type vfoType;
} __attribute__((packed));

class Radio3 {
    UsbVCom &usbVCom;
    Board &board;

    State state;
    Configuration configuration;

    DataLink dataLink;
    FreqMeter fMeter;
    AdcProbes adcProbes;
    Ad985x vfo;
    Sweep sweep;

    void sendFrame(FrameCmd cmd, const void *payload, uint16_t size);
    void vfoRelayCommit();
    void vfoToVna();
    void vfoToSocket();
    void adjustVfoOutputFor(Sweep::Source source);
    void sendPing();
    void sendDeviceConfiguration();
    void sendDeviceState();
    void sendLicenseData();
    void cmdSetVfoFrequency(const uint8_t *payload);
    void cmdGetVfoFrequency();
    void cmdSweepRequest(const uint8_t *payload);
    void cmdVfoType(const uint8_t *payload);
    void cmdVfoAttenuator(const uint8_t *payload);
    void cmdVfoAmplifier(const uint8_t *payload);
    VnaValue readVnaProbe();
    ProbeValues readAllProbes();
    void cmdSampleAllProbes();
    void handleIncomingFrame();

public:
    explicit Radio3(UsbVCom &usbVCom1, Board &board);
    void init();
    void start();
    void sysTick();
};