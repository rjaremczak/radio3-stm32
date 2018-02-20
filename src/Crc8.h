/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include <cstdint>

class Crc8 {
    uint8_t crc = 0;

public:
    void process(uint8_t byte);
    void process(const uint8_t *buf, uint16_t size);

    inline uint8_t value() { return crc; }
};