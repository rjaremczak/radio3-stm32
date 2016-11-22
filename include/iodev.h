//
// Created by Robert Jaremczak on 2016.11.21.
//

#ifndef RADIO3_STM32_IODEV_H
#define RADIO3_STM32_IODEV_H

#include <stdint.h>

// methods must be provided by a concrete implementation

void iodev_init(void);
uint8_t iodev_canRead(void);
uint8_t iodev_error(void);
void iodev_write(uint8_t);
uint8_t iodev_read(void);

// methods implemented in generic way

uint16_t iodev_readWord(void);
void iodev_readBuf(uint8_t *, uint16_t);
void iodev_writeWord(uint16_t);
void iodev_writeBuf(uint8_t *, uint16_t);

#endif //RADIO3_STM32_IODEV_H
