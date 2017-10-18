/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _RADIO3_H_
#define _RADIO3_H_

#include "DataLink.h"
#include "FreqMeter.h"
#include "AdcProbes.h"

namespace {
    const auto DEFAULT_AVG_SAMPLES = 3;

    const auto MAX_PAYLOAD_SIZE = 16;

    const auto SWEEP_HEADER_SIZE = 12;
    const auto SWEEP_MAX_STEPS = 1000;
    const auto SWEEP_MAX_SERIES = 2;
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

enum class HardwareRevision : uint8_t {
    AUTODETECT, VERSION_1, VERSION_2
};

enum class SweepSignalSource : uint8_t {
    LOG_PROBE, LIN_PROBE, VNA
};

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

struct Complex {
    uint16_t value;
    uint16_t phase;
} __packed;

struct ProbeValues {
    uint16_t logarithmic;
    uint16_t linear;
    Complex complex;
    uint32_t fMeter;
} __packed;

class Radio3 {
    ComDevice &comDevice;
    DataLink dataLink;
    FreqMeter fMeter;
    AdcProbes adcProbes;

    void sendFrame(FrameCmd cmd, void *payload, uint16_t size);
    void sendSweepResponse();
    void vfoRelayCommit();
    void vfoOutput_vna();
    void vfoOutput_direct();
    void vfoRelay_set(SweepSignalSource source);
    void resetSweepData(uint16_t totalSamples);
    void divideAccumulatedData(uint16_t totalSamples, uint8_t divider);
    void sweepAndAccumulate(uint16_t totalSamples, uint8_t avgSamples);
    void performSweep(SweepRequest *req);
    void sendPing();
    void sendDeviceInfo();
    void sendDeviceState();
    void cmdSetVfoFrequency(const uint8_t *payload);
    void cmdGetVfoFrequency();
    void cmdSweepStart(uint8_t *payload);
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
    explicit Radio3(ComDevice &comDevice);
    void init();
    void start();
    void sysTick();
};

#endif
