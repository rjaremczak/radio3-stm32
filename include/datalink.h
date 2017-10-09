/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */


#ifndef _DATALINK_H_
#define _DATALINK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define DATALINK_STATUS_OK		0
#define DATALINK_CRC_ERROR		1
#define DATALINK_IODEV_ERROR	2

struct DataLinkFrame {
	uint8_t format;
	uint16_t command;
	uint16_t payloadSize;
};

void datalink_init();
void datalink_writeFrame(uint16_t type, void *payload, uint16_t size);
void datalink_readFrame(DataLinkFrame *frame, uint8_t *payloadBuf, uint16_t maxPayloadSize);
bool datalink_isIncomingData();
bool datalink_error();

#ifdef __cplusplus
}
#endif

#endif
