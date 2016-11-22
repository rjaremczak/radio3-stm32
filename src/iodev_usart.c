/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Serial communication library
 *
 */

#include "iodev.h"
#include <stm32f10x.h>

#define READ_TIMEOUT_MS 200
#define USART USART3
#define USART_STATUS_OK		0
#define USART_TIMEOUT		2

extern volatile uint32_t currentTime;

static uint8_t status = USART_STATUS_OK;
static uint32_t timeout;

void iodev_init(void) {

	// enable clock for IO components

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // TX as output

    GPIO_InitTypeDef gpioInitData;
    GPIO_StructInit(&gpioInitData);
    gpioInitData.GPIO_Pin = GPIO_Pin_10;
    gpioInitData.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &gpioInitData);

    // RX as input

    gpioInitData.GPIO_Pin = GPIO_Pin_11;
    gpioInitData.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpioInitData);

    // USART configuration

    USART_DeInit(USART);
    USART_InitTypeDef usartInitData;
    USART_StructInit(&usartInitData);
    usartInitData.USART_BaudRate = 115200;
    USART_Init(USART, &usartInitData);
    USART_Cmd(USART, ENABLE);
}

inline uint8_t iodev_can_read(void) {
	return USART_GetFlagStatus(USART, USART_FLAG_RXNE) != RESET;
}

inline uint8_t iodev_error(void) {
	return status != USART_STATUS_OK;
}

void iodev_write(uint8_t byte) {
    USART_SendData(USART, byte);
    while (USART_GetFlagStatus(USART, USART_FLAG_TXE) == RESET) {}
}

uint8_t iodev_read(void) {
	timeout = currentTime + READ_TIMEOUT_MS;
    while (USART_GetFlagStatus(USART, USART_FLAG_RXNE) == RESET) {
    	if(currentTime > timeout) {
    		status = USART_TIMEOUT;
    		return 0;
    	}
    }
    status = USART_STATUS_OK;
    return USART_ReceiveData(USART);
}

uint16_t usart_readWord(void) {
    uint8_t low = usart_readbyte();
    if(status != USART_STATUS_OK) { return 0; }

    uint8_t high = usart_readbyte();
    if(status != USART_STATUS_OK) { return 0; }

    return (high << 8) + low;
}

void usart_writeWord(uint16_t word) {
    usart_writebyte(word & 0xFF);
    usart_writebyte((word >> 8) & 0xFF);
}

void usart_writeBuf(uint8_t *buf, uint16_t size) {
    while(size--) {
        usart_writebyte(*buf++);
    }
}

void usart_readBuf(uint8_t *buf, uint16_t size) {
    while(size--) {
        *buf++ = usart_readbyte();
        if(status != USART_STATUS_OK) { return; }
    }
}

