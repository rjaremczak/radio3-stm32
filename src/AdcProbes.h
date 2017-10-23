/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include <cstdint>

class AdcProbes {
    uint16_t read(uint8_t channel, uint8_t avgSamples);
    uint16_t read(uint8_t channel);

public:
    void init();
    uint16_t readLogarithmic(uint8_t avgSamples);
    uint16_t readLinear(uint8_t avgSamples);
    uint16_t readVnaGain(uint8_t avgSamples);
    uint16_t readVnaPhase(uint8_t avgSamples);
};