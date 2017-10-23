/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include <cstdint>

class Vfo {
public:
    enum class Type : uint8_t {
        NONE, DDS_AD9850, DDS_AD9851
    };

    virtual void setFrequency(uint32_t frequency) = 0;
    virtual uint32_t frequency() = 0;
};
