/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Implementation of data-link layer protocol
 * communication between uC unit and desktop software.
 * Uses serial communication with CRC-8 based corruption detection
 *
 */

#include "datalink.h"
#include "iodev.h"
#include "crc.h"

static volatile uint8_t status = DATALINK_STATUS_OK;

static void write_word(uint8_t *crc, uint16_t word) {
    iodev_write_word(word);
	crc8_word(crc, word);
}

static void write_byte(uint8_t *crc, uint8_t byte) {
    iodev_write_word(byte);
	crc8_word(crc, byte);
}

static uint16_t read_word(uint8_t *crc) {
	uint16_t word = iodev_read_word();
	if(iodev_error()) { return 0; }

	crc8_word(crc, word);
	return word;
}

static uint8_t read_byte(uint8_t *crc) {
	uint8_t byte = iodev_read();
	if(iodev_error()) { return 0; }

	crc8_byte(crc, byte);
	return byte;
}

void datalink_init(void) {
}

inline uint8_t datalink_error(void) {
	return status != DATALINK_STATUS_OK;
}

int datalink_writeFrame(uint16_t type, void *payload, uint16_t size) {
	uint8_t crc = 0;
	if(size <= 13) {
		write_word(&crc, (size << 12) | type);
	} else if(size <= 269) {
		write_word(&crc, (14 << 12) | type);
		write_byte(&crc, size-14);
	} else {
		write_word(&crc, (15 << 12) | type);
		write_word(&crc, size - 270);
	}

	if(size > 0) {
        iodev_write_buf(payload, size);
		crc8_buf(&crc, payload, size);
	}
	iodev_write(crc);
	return 0;
}

void datalink_readFrame(struct datalink_frame *frame, uint8_t *payloadBuf, uint16_t maxPayloadSize) {
	status = DATALINK_STATUS_OK;

	uint8_t crc = 0;
	uint16_t header = read_word(&crc);
	if(iodev_error()) {
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
	if(iodev_error()) {
		status = DATALINK_IODEV_ERROR;
		return;
	}

	uint16_t size = frame->payloadSize > maxPayloadSize ? maxPayloadSize : frame->payloadSize;
	if(size > 0) {
        iodev_read_buf(payloadBuf, size);
		if(iodev_error()) { return; }
		crc8_buf(&crc, payloadBuf, size);
	}
	uint8_t receivedCrc = iodev_read();
	if(iodev_error()) {
		status = DATALINK_IODEV_ERROR;
		return;
	}

	if(crc != receivedCrc) {
		status = DATALINK_CRC_ERROR;
		return;
	}

	status = DATALINK_STATUS_OK;
}

inline uint8_t datalink_isIncomingData(void) {
	return iodev_read_data_ready();
}
