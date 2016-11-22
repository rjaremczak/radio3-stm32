/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef RADIO3_STM32_USART_H_
#define RADIO3_STM32_USART_H_

#define USART_STATUS_OK		0
#define USART_TIMEOUT		2

#include <stm32f10x.h>

void usart_init(void);
void usart_writebyte(uint8_t);
uint8_t usart_readbyte(void);
uint16_t usart_readword(void);
void usart_writeword(uint16_t);
void usart_writebuf(uint8_t *, uint16_t);
void usart_readbuf(uint8_t *, uint16_t);
uint8_t usart_isReadDataReady(void);
uint8_t usart_error(void);

#endif
