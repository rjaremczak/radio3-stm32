/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _CRC_H_
#define _CRC_H_

#include <cstdint>

class Crc8 {
    uint8_t crc = 0;
public:
    void process(uint8_t byte);
    void process(const uint8_t *buf, uint16_t size);
    uint8_t value();
};

#endif
