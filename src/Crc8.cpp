/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * CRC8 functions used by data-link
 */

#include "Crc8.h"

void Crc8::process(uint8_t byte) {
    for (uint8_t tempI = 8; tempI; tempI--) {
        auto sum = static_cast<uint8_t>((crc ^ byte) & 0x01);
        crc >>= 1;
        if (sum) {
            crc ^= 0x8C;
        }
        byte >>= 1;
    }
}

void Crc8::process(const uint8_t *buf, uint16_t size) {
    while (size--) process(*(buf++));
}

uint8_t Crc8::value() {
    return crc;
}
