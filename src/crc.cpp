/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * CRC8 functions used by data-link
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "crc.h"

void crc8_byte(uint8_t *crc, uint8_t data) {
	for (uint8_t tempI = 8; tempI; tempI--) {
	  uint8_t sum = (uint8_t) ((*crc ^ data) & 0x01);
	  *crc >>= 1;
	  if (sum) {
		*crc ^= 0x8C;
	  }
	  data >>= 1;
	}
}

void crc8_word(uint8_t *crc, uint16_t word) {
	crc8_byte(crc, (uint8_t) (word & 0xff));
	crc8_byte(crc, (uint8_t) (word >> 8));
}

void crc8_buf(uint8_t *crc, void *buf, uint16_t size) {
    uint8_t *bytePtr = (uint8_t *)buf;
	while(size--) {
		crc8_byte(crc, *(bytePtr++));
	}
}

#ifdef __cplusplus
}
#endif

