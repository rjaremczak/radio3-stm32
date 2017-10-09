//
// Created by Robert Jaremczak on 2017.10.04.
//

#ifndef RADIO3_STM32_USBVCOM_H
#define RADIO3_STM32_USBVCOM_H


#include <Timer.h>
#include <UsbDevice.h>

namespace {
    const auto BLK_BUFF_SIZE = 64;
    const auto IO_TIMEOUT_MS = 200;
}

class UsbVCom : public UsbDevice {
    enum class Status { OK, TIMEOUT };

    struct Buffer {
        uint8_t data[BLK_BUFF_SIZE]{};
        volatile uint8_t inPos = 0;
        volatile uint8_t outPos = 0;
        void clear();
        uint8_t count();
        bool empty();
        bool full();
        uint8_t get();
        void put(uint8_t byte);
    };

    const Timer& timer;
    volatile Status status = Status::OK;
    volatile bool writeInProgress = false;
    Buffer rxBuf;
    Buffer txBuf;

    void clearError();
    void readPacket();

protected:
    void ep3out() override;
    void ep1in() override;

public:
    explicit UsbVCom(Timer& timer);
    void init();
    bool error();

    uint8_t available();
    uint8_t read();
    void read(uint8_t *buf, uint16_t size);

    void write(uint8_t byte);
    void write(uint8_t *buf, uint16_t size);
    void flush();
};

#endif //RADIO3_STM32_USBVCOM_H
