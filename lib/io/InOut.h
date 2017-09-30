//
// Created by Robert Jaremczak on 2017.09.30.
//

#ifndef RADIO3_STM32_INOUT_H
#define RADIO3_STM32_INOUT_H

#include <cstdint>

class InOut {
    enum Status {
        OK, INCOMPLETE, TIMEOUT
    };

public:
    virtual uint8_t available() = 0;
    virtual Status read(uint8_t *buf, uint16_t size) = 0;
    virtual Status write(const uint8_t *buf, uint16_t size) = 0;
    virtual bool idle();
};


#endif //RADIO3_STM32_INOUT_H
