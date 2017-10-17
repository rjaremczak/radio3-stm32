//
// Created by Robert Jaremczak on 2017.10.05.
//

#ifndef RADIO3_STM32_USBDEVICE_H
#define RADIO3_STM32_USBDEVICE_H


#include <cstdint>
#include "usb_lib.h"

class UsbDevice {
protected:
    void initRCC();
    void initNVIC();
    void intToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);

    void (UsbDevice::*epIn[7])();
    void (UsbDevice::*epOut[7])();

    virtual void ep1in();
    virtual void ep2in();
    virtual void ep3in();
    virtual void ep4in();
    virtual void ep5in();
    virtual void ep6in();
    virtual void ep7in();

    virtual void ep1out();
    virtual void ep2out();
    virtual void ep3out();
    virtual void ep4out();
    virtual void ep5out();
    virtual void ep6out();
    virtual void ep7out();

    virtual void handleStartOfFrame();
    virtual void handleCorrectTransfer();
    virtual void handleReset();
    virtual void handleError();
    virtual void handleWakeUp();
    virtual void handleSuspend();
    virtual void handleExpectedStartOfFrame();

public:
    UsbDevice();
    void istr();
};

#endif //RADIO3_STM32_USBDEVICE_H
