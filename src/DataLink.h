/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */


#ifndef _DATALINK_H_
#define _DATALINK_H_

#include <cstdint>

class DataLink {
    enum class Status : uint8_t {
        OK = 0,
        CRC_ERROR = 1,
        IO_ERROR = 2
    };

    Status status = Status::OK;
    UsbVCom &usbVCom;

    void writeByte(uint8_t *crc, uint8_t byte);
    void writeWord(uint8_t *crc, uint16_t word);
    uint8_t readByte(uint8_t *crc);
    uint16_t readWord(uint8_t *crc);

public:
    struct Frame {
        uint8_t format;
        uint16_t command;
        uint16_t payloadSize;
    };

    explicit DataLink(UsbVCom &usbVCom);
    void writeFrame(uint16_t type, uint8_t *payload, uint16_t size);
    void readFrame(Frame *frame, uint8_t *payloadBuf, uint16_t maxPayloadSize);
    bool isIncomingData();
    bool error();
};



#endif
