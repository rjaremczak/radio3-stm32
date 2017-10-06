//
// Created by Robert Jaremczak on 2017.10.04.
//

#ifndef RADIO3_STM32_USBVCOM_H
#define RADIO3_STM32_USBVCOM_H


#include <Timer.h>
#include <UsbDevice.h>

namespace {
    const auto BLK_BUFF_SIZE = 64;
}

class UsbVCom : public UsbDevice {
    const Timer& timer;
    uint8_t rxBuf[BLK_BUFF_SIZE];
    volatile uint8_t rxBytes = 0;

protected:
    void ep3out() override;
    void ep1in() override;

public:
    UsbVCom(Timer& timer);
    void init();
};


#endif //RADIO3_STM32_USBVCOM_H
