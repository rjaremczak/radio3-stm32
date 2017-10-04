//
// Created by Robert Jaremczak on 2017.10.04.
//

#ifndef RADIO3_STM32_USBVCOM_H
#define RADIO3_STM32_USBVCOM_H


#include <Timer.h>

namespace {
    const auto BLK_BUFF_SIZE = 64;
}

class UsbVCom {
    const Timer& timer;
    uint8_t rxBuf[BLK_BUFF_SIZE];
    volatile uint8_t rxBufSize = 0;

public:
    UsbVCom(Timer& timer);
    void init();

    void ep3out();
    void ep1in();
    void sof();

};


#endif //RADIO3_STM32_USBVCOM_H
