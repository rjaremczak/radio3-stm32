//
// Created by Robert Jaremczak on 2017.10.22.
//

#pragma once

#include <cstdint>
#include "Vfo.h"
#include "AdcProbes.h"

namespace {
    const auto SWEEP_HEADER_SIZE = 12;
    const auto SWEEP_MAX_STEPS = 1000;
    const auto SWEEP_MAX_SERIES = 2;
}

class Sweep {
public:
    enum class Source : uint8_t {
        LOG_PROBE, LIN_PROBE, VNA
    };

    enum class State : uint8_t {
        READY, PROCESSING, INVALID_REQUEST
    };

    struct Request {
        uint32_t freqStart;
        uint32_t freqStep;
        uint16_t steps;
        Source source;
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

    } __attribute__((packed));

    struct Response {
        State state;
        uint32_t freqStart;
        uint32_t freqStep;
        uint16_t steps;
        Source source;
        uint16_t data[SWEEP_MAX_SERIES * (SWEEP_MAX_STEPS + 1)];

        uint16_t totalSamples() const {
            const auto ns = (uint16_t) (steps + 1);
            return (uint16_t) (source == Source::VNA ? ns * 2 : ns);
        }

        uint16_t size() const {
            return SWEEP_HEADER_SIZE + totalSamples() * sizeof(uint16_t);
        }
    } __attribute__((packed));

private:
    Vfo &vfo;
    AdcProbes &adcProbes;
    Response response;
    State state = State::READY;

    void resetData(uint16_t length);
    void divideData(uint16_t length, uint8_t divider);
    void accumulateData(uint16_t length, uint8_t avgSamples);

public:
    Sweep(Vfo &vfo, AdcProbes &adcProbes);
    void perform(Request &request);
    void init();

    inline State getState() { return state; }
    inline const Response &getResponse() { return response; }
};