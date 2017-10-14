/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Implementation of data-link layer protocol
 * communication between uC unit and desktop software.
 * Uses serial communication with CRC-8 based corruption detection
 *
 */

#include <UsbVCom.h>
#include "DataLink.h"
#include "crc.h"

bool DataLink::error() {
	return status != Status::OK;
}

void DataLink::writeFrame(uint16_t type, uint8_t *payload, uint16_t size) {
	uint8_t crc = 0;
	if(size <= 13) {
		writeWord(&crc, (size << 12) | type);
	} else if(size <= 269) {
		writeWord(&crc, (uint16_t) ((14 << 12) | type));
		writeByte(&crc, (uint8_t) (size - 14));
	} else {
		writeWord(&crc, (uint16_t) ((15 << 12) | type));
		writeWord(&crc, (uint16_t) (size - 270));
	}
    if(usbVCom.error()) return;

	if(size > 0) {
		usbVCom.write(payload, size);
        if(usbVCom.error()) return;
		crc8_buf(&crc, payload, size);
	}

	usbVCom.write(crc);
	usbVCom.flush();
}

void DataLink::readFrame(Frame *frame, uint8_t *payloadBuf, uint16_t maxPayloadSize) {
	status = Status::OK;

	uint8_t crc = 0;
	uint16_t header = readWord(&crc);
	if(usbVCom.error()) {
		status = Status::IO_ERROR;
		return;
	}

	frame->format = static_cast<uint8_t>((header >> 12) & 0x0f);
	frame->command = static_cast<uint16_t>(header & 0x7ff);

	if(frame->format <= 13) {
		frame->payloadSize = frame->format;
	} else if(frame->format == 14) {
		frame->payloadSize = static_cast<uint16_t>(readByte(&crc) + 14);
	} else {
		frame->payloadSize = static_cast<uint16_t>(readWord(&crc) + 270);
	}
	if(usbVCom.error()) {
		status = Status::IO_ERROR;
		return;
	}

	uint16_t size = frame->payloadSize > maxPayloadSize ? maxPayloadSize : frame->payloadSize;
	if(size > 0) {
        usbVCom.read(payloadBuf, size);
		if(usbVCom.error()) { return; }
		crc8_buf(&crc, payloadBuf, size);
	}
	uint8_t receivedCrc = usbVCom.read();
	if(usbVCom.error()) {
		status = Status::IO_ERROR;
		return;
	}

	if(crc != receivedCrc) {
		status = Status::CRC_ERROR;
		return;
	}

	status = Status::OK;
}

bool DataLink::isIncomingData() {
    return usbVCom.available()>0;
}
DataLink::DataLink(UsbVCom &usbVCom) : usbVCom(usbVCom) {}

void DataLink::writeByte(uint8_t *crc, uint8_t byte) {
    if(usbVCom.error()) return;

    usbVCom.write(byte);
    crc8_byte(crc, byte);
}

void DataLink::writeWord(uint8_t *crc, uint16_t word) {
    writeByte(crc, (uint8_t) (word & 0xFF));
    writeByte(crc, (uint8_t) ((word >> 8) & 0xFF));
}

uint8_t DataLink::readByte(uint8_t *crc) {
    if(usbVCom.error()) return 0;

    uint8_t byte = usbVCom.read();
    crc8_byte(crc, byte);
    return byte;
}

uint16_t DataLink::readWord(uint8_t *crc) {
    uint8_t low = readByte(crc);
    uint8_t high = readByte(crc);
    return (high << 8) + low;
}

