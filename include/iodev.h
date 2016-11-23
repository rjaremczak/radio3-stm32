//
// Created by Robert Jaremczak on 2016.11.21.
//

#ifndef RADIO3_STM32_IODEV_H
#define RADIO3_STM32_IODEV_H

#include <stdint.h>

// methods must be provided by a concrete implementation

void iodev_init(void);
uint8_t iodev_read_data_ready(void);
uint8_t iodev_error(void);
void iodev_write(uint8_t);
uint8_t iodev_read(void);

// methods implemented in generic way

uint16_t iodev_read_word(void);
void iodev_read_buf(uint8_t *, uint16_t);
void iodev_write_word(uint16_t);
void iodev_write_buf(uint8_t *, uint16_t);

#endif //RADIO3_STM32_IODEV_H
