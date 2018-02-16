/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include <cstdint>

class Vfo {
public:
    enum class Type : uint8_t {
        ddsAD9850, ddsAD9851
    };

    virtual void setFrequency(uint32_t frequency) = 0;
    virtual uint32_t frequency() = 0;
};
