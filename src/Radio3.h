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
    PING = 0x000,
    DEVICE_INFO = 0x001,
    DEVICE_STATE = 0x002,
    DEVICE_HARDWARE_REVISION = 0x003,
    VFO_GET_FREQ = 0x008,
    VFO_SET_FREQ = 0x009,
    LOGPROBE_DATA = 0x010,
    LINPROBE_DATA = 0x018,
    CMPPROBE_DATA = 0x020,
    FMETER_DATA = 0x028,
    PROBES_DATA = 0x030,
    VFO_OUT_DIRECT = 0x033,
    VFO_OUT_VNA = 0x034,
    VFO_TYPE = 0x035,
    VFO_ATTENUATOR = 0x036,
    VFO_AMPLIFIER = 0x037,
    SWEEP_REQUEST = 0x040,
    SWEEP_RESPONSE = 0x041
};

struct Complex {
    uint16_t value;
    uint16_t phase;
} __attribute__((packed));

struct ProbeValues {
    uint16_t logarithmic;
    uint16_t linear;
    Complex complex;
    uint32_t fMeter;
} __attribute__((packed));

class Radio3 {
    UsbVCom &usbVCom;
    Board &board;

    DataLink dataLink;
    FreqMeter fMeter;
    AdcProbes adcProbes;
    Ad985x vfo;
    Sweep sweep;

    void sendFrame(FrameCmd cmd, const void *payload, uint16_t size);
    void vfoRelayCommit();
    void vfoOutput_vna();
    void vfoOutput_direct();
    void vfoRelay_set(Sweep::Source source);
    void sendPing();
    void sendDeviceInfo();
    void sendDeviceState();
    void cmdSetVfoFrequency(const uint8_t *payload);
    void cmdGetVfoFrequency();
    void cmdSweepRequest(uint8_t *payload);
    void cmdSampleFMeter();
    void cmdSampleLogarithmicProbe();
    void cmdSampleLinearProbe();
    void cmdVfoType(const uint8_t *payload);
    void cmdVfoAttenuator(const uint8_t *payload);
    void cmdVfoAmplifier(const uint8_t *payload);
    Complex readVnaProbe();
    ProbeValues readAllProbes();
    void cmdSampleComplexProbe();
    void cmdSampleAllProbes();
    void cmdHardwareRevision(HardwareRevision hardwareRevision);
    void handleIncomingFrame();

public:
    explicit Radio3(UsbVCom &usbVCom1, Board &board);
    void init();
    void start();
    void sysTick();
};