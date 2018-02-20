/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */


#pragma once

#include <cstdint>
#include "Crc8.h"

class DataLink {
    enum class Status : uint8_t {
        OK = 0,
        CRC_ERROR = 1,
        IO_ERROR = 2
    };

    Status status = Status::OK;
    UsbVCom &usbVCom;

    void writeByte(Crc8 &crc, uint8_t byte);
    void writeWord(Crc8 &crc, uint16_t word);
    uint8_t readByte(Crc8 &crc);
    uint16_t readWord(Crc8 &crc);

public:
    struct Frame {
        uint8_t format;
        uint16_t command;
        uint16_t payloadSize;
    };

    explicit DataLink(UsbVCom &comDevice);
    void writeFrame(uint16_t type, const uint8_t *payload, uint16_t size);
    void readFrame(Frame *frame, uint8_t *payloadBuf, uint16_t maxPayloadSize);

    inline bool isIncomingData() { return usbVCom.available() > 0; }
    inline bool error() { return status != Status::OK; }
};
