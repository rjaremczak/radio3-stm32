/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _CRC_H_
#define _CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void crc8_byte(uint8_t *, uint8_t);
void crc8_word(uint8_t *, uint16_t);
void crc8_buf(uint8_t *, uint8_t *, uint16_t);

#ifdef __cplusplus
}
#endif

#endif
