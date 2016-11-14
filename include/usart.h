/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _USART_H_
#define _USART_H_

#define USART_STATUS_OK		0
#define USART_CRC_ERROR		1
#define USART_TIMEOUT		2

#include <stm32f10x.h>

void usart_init(void);
void usart_writebyte(uint8_t);
uint8_t usart_readbyte(void);
uint16_t usart_readword(void);
void usart_writeword(uint16_t);
void usart_writebuf(uint8_t *, uint16_t);
void usart_readbuf(uint8_t *, uint16_t);
void usart_read_wait(void);
uint8_t usart_isReadDataReady(void);
uint8_t usart_error(void);
uint8_t usart_statusCode(void);

#endif
