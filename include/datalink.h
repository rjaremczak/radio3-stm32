/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _DATALINK_H_
#define _DATALINK_H_

#include <stdint.h>

#define DATALINK_STATUS_OK		0
#define DATALINK_CRC_ERROR		1
#define DATALINK_IODEV_ERROR	2

struct datalink_frame {
	uint8_t format;
	uint16_t command;
	uint16_t payloadSize;
};

void datalink_init(void);
int datalink_writeFrame(uint16_t type, void *payload, uint16_t size);
void datalink_readFrame(struct datalink_frame *frame, uint8_t *payloadBuf, uint16_t maxPayloadSize);
uint8_t datalink_isIncomingData(void);
uint8_t datalink_error(void);
uint8_t datalink_statusCode(void);

#endif
