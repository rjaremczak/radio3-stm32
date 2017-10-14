//
// Created by Robert Jaremczak on 2017.10.05.
//

#ifndef RADIO3_STM32_USBDEVICE_H
#define RADIO3_STM32_USBDEVICE_H


#include <cstdint>

class UsbDevice {
protected:
    void initClock();
    void initInterrupts();

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

    // virtual void init();
    // virtual void reset();
    // virtual void processStatusIn();
    // virtual void processStatusOut();
    // virtual RESULT classDataSetup(uint8_t requestNo);
    // virtual RESULT classNoDataSetup(uint8_t requestNo);
    // virtual RESULT classGetInterfaceSetting(uint8_t interface, uint8_t alternateSetting);
    // virtual uint8_t *getDeviceDescriptor();
    // virtual uint8_t *getConfigDescriptor();
    // virtual uint8_t *getStringDescriptor();
    // virtual uint8_t getMaxPacketSize();


public:
    UsbDevice();
    void ctrLp();
};

#endif //RADIO3_STM32_USBDEVICE_H
