//
// Created by Robert Jaremczak on 2017.10.22.
//

#include "Sweep.h"
#include "delay.h"

Sweep::Sweep(Vfo &vfo, AdcProbes &adcProbes) : vfo(vfo), adcProbes(adcProbes) {}

void Sweep::resetData(uint16_t length) {
    for (uint16_t i = 0; i < length; i++) { response.data[i] = 0; }
}

void Sweep::divideData(uint16_t length, uint8_t divider) {
    for (uint16_t step = 0; step < length; step++) { response.data[step] /= divider; }
}

void Sweep::accumulateData(uint16_t length, uint8_t avgSamples) {
    uint32_t freq = response.freqStart;
    uint16_t sampleNo = 0;
    while (sampleNo < length) {
        vfo.setFrequency(freq);
        delayUs(5);
        switch (response.source) {
            case Source::LOG_PROBE:
                response.data[sampleNo++] += adcProbes.readLogarithmic(avgSamples);
                break;
            case Source::LIN_PROBE:
                response.data[sampleNo++] += adcProbes.readLinear(avgSamples);
                break;
            case Source::VNA:
                response.data[sampleNo++] += adcProbes.readVnaGain(avgSamples);
                response.data[sampleNo++] += adcProbes.readVnaPhase(avgSamples);
                break;
        }
        freq += response.freqStep;
    }
}

void Sweep::perform(Request &request) {
    if(!request.isValid() || request.steps >= SWEEP_MAX_STEPS) {
        response.steps = 0;
        response.state = Sweep::State::INVALID_REQUEST;
        return;
    }

    state = Sweep::State::PROCESSING;
    response.freqStart = request.freqStart;
    response.freqStep = request.freqStep;
    response.steps = request.steps;
    response.source = request.source;

    const auto avgPasses = request.getAvgPasses();
    const auto avgSamples = request.getAvgSamples();
    const auto totalSamples = response.totalSamples();

    auto prevFreq = vfo.frequency();
    accumulateData(totalSamples >> 4, avgSamples);
    resetData(totalSamples);
    for (auto i=0; i < avgPasses; i++) { accumulateData(totalSamples, avgSamples); }
    divideData(totalSamples, avgPasses);
    vfo.setFrequency(prevFreq);
    state = Sweep::State::READY;
}

Sweep::State Sweep::getState() {
    return state;
}

const Sweep::Response &Sweep::getResponse() {
    return response;
}

void Sweep::init() {
    state = Sweep::State::READY;
}
