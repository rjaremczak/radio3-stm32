//
// Created by Robert Jaremczak on 2017.10.04.
//

#ifndef RADIO3_STM32_USBVCOM_H
#define RADIO3_STM32_USBVCOM_H


#include <Timer.h>
#include <UsbDevice.h>

namespace {
    const auto RX_BUF_SIZE = 64;
    const auto TX_BUF_SIZE = 63;
    const auto IO_TIMEOUT_MS = 200;
}

class UsbVCom : public UsbDevice {
    enum class Status {
        OK, TIMEOUT
    };

    template<int S>
    struct Buffer {
        uint8_t data[S]{};
        volatile uint8_t inPos = 0;
        volatile uint8_t outPos = 0;
        void clear();
        uint8_t count();
        bool empty();
        bool full();
        uint8_t get();
        void put(uint8_t byte);
    };

    const Timer &timer;
    Status status = Status::OK;
    volatile bool writeInProgress = false;
    Buffer<RX_BUF_SIZE> rxBuf;
    Buffer<TX_BUF_SIZE> txBuf;

    void clearError();
    void readPacket();

protected:
    void ep3out() override;
    void ep1in() override;

public:
    explicit UsbVCom(Timer &timer);
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
