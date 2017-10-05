//
// Created by Robert Jaremczak on 2017.10.05.
//

#ifndef RADIO3_STM32_USBDEVICE_H
#define RADIO3_STM32_USBDEVICE_H


#include <cstdint>

class UsbDevice {
protected:
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

public:
    UsbDevice();
    void ctrLp();
};


#endif //RADIO3_STM32_USBDEVICE_H
