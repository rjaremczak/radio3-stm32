//
// Created by Robert Jaremczak on 2017.10.04.
//

#ifndef RADIO3_STM32_COMDEVICE_H
#define RADIO3_STM32_COMDEVICE_H


#include <cstdint>

class ComDevice {
public:
    virtual bool error() = 0;
    virtual uint8_t available() = 0;
    virtual uint8_t read() = 0;
    virtual void read(uint8_t *buf, uint16_t size) = 0;
    virtual void write(uint8_t byte) = 0;
    virtual void write(uint8_t *buf, uint16_t size) = 0;
    virtual void flush() = 0;
};

#endif
