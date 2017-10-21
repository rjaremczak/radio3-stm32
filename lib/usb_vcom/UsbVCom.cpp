//
// Created by Robert Jaremczak on 2017.10.04.
//

#include <log.h>
#include "UsbVCom.h"

extern uint8_t Request;

UsbVCom::UsbVCom(Timer &timer) : UsbDevice(), timer(timer) {
}

void UsbVCom::init() {
    initRCC();
    initNVIC();
    USB_Init();
    log("USB VCOM initialized");
}

void UsbVCom::ep3out() {
    if(rxBuf.empty()) {
        uint16_t count = GetEPRxCount(EP3_OUT & 0x7F);
        PMAToUserBufferCopy(rxBuf.data, GetEPRxAddr(EP3_OUT & 0x7F), count);
        rxBuf.inPos = count;
        log("ep3out: received %d B", rxBuf.count());
    }
}

void UsbVCom::ep1in() {
    log("ep1in: sent %d B", txBuf.count());
    txBuf.clear();
    writeInProgress = false;
}

uint8_t UsbVCom::available() {
    return rxBuf.count();
}

bool UsbVCom::error() {
    return status != Status::OK;
}

void UsbVCom::clearError() {
    status = Status::OK;
}

void UsbVCom::readPacket() {
    rxBuf.clear();
    SetEPRxValid(ENDP3);

    log("waiting for packet ...");

    auto time = timer.getMillis() + IO_TIMEOUT_MS;
    while(rxBuf.empty()) {
        if(timer.isAfter(time)) {
            status = Status::TIMEOUT;
            break;
        }
    };

    log("received %d B, status: %d\n", rxBuf.count(), status);
}

uint8_t UsbVCom::read() {
    clearError();
    if(rxBuf.empty()) {
        log("rxBuf empty, read more...");
        readPacket();
    }
    if(error()) return 0;

    uint8_t byte = rxBuf.get();
    if(rxBuf.empty()) {
        log("rxBuf empty, schedule next read");
        rxBuf.clear();
        SetEPRxValid(ENDP3);
    }

    return byte;
}

void UsbVCom::read(uint8_t *buf, uint16_t size) {
    log("read %d B", txBuf.count());
    while (size-- > 0 && !error()) { *buf++ = read(); }
}

void UsbVCom::write(uint8_t byte) {
    if(txBuf.full()) {
        flush();
        txBuf.put(byte);
    } else {
        txBuf.put(byte);
        if(txBuf.full()) flush();
    }
}

void UsbVCom::write(uint8_t *buf, uint16_t size) {
    log("write %d B", txBuf.count());
    while(size-- > 0 && !error()) { write(*buf++); }
}

void UsbVCom::flush() {
    clearError();
    log("flush %d B", txBuf.count());
    writeInProgress = true;
    UserToPMABufferCopy(txBuf.data, ENDP1_TXADDR, txBuf.count());
    SetEPTxCount(ENDP1, txBuf.count());
    SetEPTxValid(ENDP1);

    auto time = timer.getMillis() + IO_TIMEOUT_MS;
    while (writeInProgress) {
        if(timer.isAfter(time)) {
            status = Status::TIMEOUT;
            break;
        }
    };
    log("done, status: %d", status);
}

template <int S>
void UsbVCom::Buffer<S>::clear() {
    inPos = 0;
    outPos = 0;
}

template <int S>
uint8_t UsbVCom::Buffer<S>::count() {
    return inPos - outPos;
}

template <int S>
bool UsbVCom::Buffer<S>::empty() {
    return outPos >= inPos;
}

template <int S>
uint8_t UsbVCom::Buffer<S>::get() {
    return data[outPos++];
}

template <int S>
bool UsbVCom::Buffer<S>::full() {
    return inPos >= S;
}

template <int S>
void UsbVCom::Buffer<S>::put(uint8_t byte) {
    data[inPos++] = byte;
}
