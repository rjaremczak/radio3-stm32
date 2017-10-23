//
// Created by Robert Jaremczak on 2017.10.22.
//

#pragma once

#include "Vfo.h"

class Ad985x : public Vfo {
    uint8_t CONTROL_W0;
    uint32_t XTAL;
    uint32_t currentFrequency = 0;

    uint32_t calcFreq(uint32_t hz);
    void pulse(uint16_t pin);
    void sendByte(uint8_t byte);

public:
    void init(Type type);
    void setFrequency(uint32_t frequency) override;
    uint32_t frequency() override;

};