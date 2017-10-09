//
// Created by Robert Jaremczak on 2017.10.04.
//

#include <usb_lib.h>
#include <swo.h>
#include "UsbVCom.h"

UsbVCom::UsbVCom(Timer &timer) : UsbDevice(), timer(timer) {

}

void UsbVCom::init() {
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
}

void UsbVCom::ep3out() {
    SWO_printf("ep3out\n");
    if(rxBuf.empty()) {
        uint16_t count = GetEPRxCount(EP3_OUT & 0x7F);
        PMAToUserBufferCopy(rxBuf.data, GetEPRxAddr(EP3_OUT & 0x7F), count);
        rxBuf.inPos = count;
        SWO_printf("ep3out: received %d bytes\n", rxBuf.count());
    }
}

void UsbVCom::ep1in() {
    SWO_printf("ep1in: write of %d B completed\n", txBuf.count());
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

    SWO_printf("waiting for packet ...\n");

    auto time = timer.getMillis() + IO_TIMEOUT_MS;
    while(rxBuf.empty()) {
        if(timer.isAfter(time)) {
            status = Status::TIMEOUT;
            break;
        }
    };
    SWO_printf("status: %d\n", status);
}

uint8_t UsbVCom::read() {
    clearError();
    if(rxBuf.empty()) {
        SWO_printf("rxBuf is empty, request read now\n", rxBuf.inPos, rxBuf.outPos);
        readPacket();
    }
    if(error()) return 0;

    uint8_t byte = rxBuf.get();
    if(rxBuf.empty()) {
        SWO_printf("rxBuf is empty, enable next read\n");
        rxBuf.clear();
        SetEPRxValid(ENDP3);
    }

    SWO_printf("status: %d, received: %c\n", status, byte);
    return byte;
}

void UsbVCom::read(uint8_t *buf, uint16_t size) {
    while (size-- > 0 && !error()) { *buf++ = read(); }
}

void UsbVCom::write(uint8_t byte) {
    SWO_printf("write %c, txBuf.count: %d B\n", byte, txBuf.count());
    if(txBuf.full()) {
        flush();
        txBuf.put(byte);
    } else {
        txBuf.put(byte);
        if(txBuf.full()) flush();
    }
}

void UsbVCom::write(uint8_t *buf, uint16_t size) {
    while(size-- > 0 && !error()) { write(*buf++); }
}

void UsbVCom::flush() {
    clearError();
    SWO_printf("flushing %d B ...\n", txBuf.count());
    writeInProgress = true;
    //char str[] = "test\n";
    //UserToPMABufferCopy(reinterpret_cast<uint8_t *>(str), ENDP1_TXADDR, 5);
    //SetEPTxCount(ENDP1, 5);
    SWO_printf("buf:[%.*s]\n", txBuf.count(), txBuf.data);
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
    SWO_printf("status: %d\n", status);
}

void UsbVCom::Buffer::clear() {
    inPos = 0;
    outPos = 0;
}

uint8_t UsbVCom::Buffer::count() {
    return inPos - outPos;
}

bool UsbVCom::Buffer::empty() {
    return outPos >= inPos;
}

uint8_t UsbVCom::Buffer::get() {
    return data[outPos++];
}

bool UsbVCom::Buffer::full() {
    return inPos >= sizeof(data);
}

void UsbVCom::Buffer::put(uint8_t byte) {
    data[inPos++] = byte;
}
