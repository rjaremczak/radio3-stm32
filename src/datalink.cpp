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
#include "datalink.h"
#include "crc.h"

extern UsbVCom *_usbVCom;

static volatile uint8_t status = DATALINK_STATUS_OK;

static void write_byte(uint8_t *crc, uint8_t byte) {
    if(_usbVCom->error()) return;

    _usbVCom->write(byte);
    crc8_byte(crc, byte);
}

static void write_word(uint8_t *crc, uint16_t word) {
    write_byte(crc, (uint8_t) (word & 0xFF));
    write_byte(crc, (uint8_t) ((word >> 8) & 0xFF));
}

static uint8_t read_byte(uint8_t *crc) {
    if(_usbVCom->error()) return 0;

    uint8_t byte = _usbVCom->read();
    crc8_byte(crc, byte);
    return byte;
}

static uint16_t read_word(uint8_t *crc) {
    uint8_t low = read_byte(crc);
    uint8_t high = read_byte(crc);
    return (high << 8) + low;
}

void datalink_init() {
}

bool datalink_error() {
	return status != DATALINK_STATUS_OK;
}

void datalink_writeFrame(uint16_t type, void *payload, uint16_t size) {
	uint8_t crc = 0;
	if(size <= 13) {
		write_word(&crc, (size << 12) | type);
	} else if(size <= 269) {
		write_word(&crc, (uint16_t) ((14 << 12) | type));
		write_byte(&crc, (uint8_t) (size - 14));
	} else {
		write_word(&crc, (uint16_t) ((15 << 12) | type));
		write_word(&crc, (uint16_t) (size - 270));
	}
    if(_usbVCom->error()) return;

	if(size > 0) {
		_usbVCom->write((uint8_t *) payload, size);
        if(_usbVCom->error()) return;
		crc8_buf(&crc, (uint8_t *)payload, size);
	}

	_usbVCom->write(crc);
	_usbVCom->flush();
}

void datalink_readFrame(DataLinkFrame *frame, uint8_t *payloadBuf, uint16_t maxPayloadSize) {
	status = DATALINK_STATUS_OK;

	uint8_t crc = 0;
	uint16_t header = read_word(&crc);
	if(_usbVCom->error()) {
		status = DATALINK_IODEV_ERROR;
		return;
	}

	frame->format = (header >> 12) & 0x0f;
	frame->command = header & 0x7ff;

	if(frame->format <= 13) {
		frame->payloadSize = frame->format;
	} else if(frame->format == 14) {
		frame->payloadSize = read_byte(&crc) + 14;
	} else {
		frame->payloadSize = read_word(&crc) + 270;
	}
	if(_usbVCom->error()) {
		status = DATALINK_IODEV_ERROR;
		return;
	}

	uint16_t size = frame->payloadSize > maxPayloadSize ? maxPayloadSize : frame->payloadSize;
	if(size > 0) {
        _usbVCom->read(payloadBuf, size);
		if(_usbVCom->error()) { return; }
		crc8_buf(&crc, payloadBuf, size);
	}
	uint8_t receivedCrc = _usbVCom->read();
	if(_usbVCom->error()) {
		status = DATALINK_IODEV_ERROR;
		return;
	}

	if(crc != receivedCrc) {
		status = DATALINK_CRC_ERROR;
		return;
	}

	status = DATALINK_STATUS_OK;
}

bool datalink_isIncomingData() {
    return _usbVCom->available()>0;
}
