/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * LED control functions.
 *
 */

#include "stm32f10x.h"
#include "led.h"

#define LED_GPIO			GPIOC
#define LED_BOARD_PIN		GPIO_Pin_13

#define LED_BIT_ACTION(on)	(on ? Bit_RESET : Bit_SET)

void led_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef gpi;
	gpi.GPIO_Pin = LED_BOARD_PIN;
	gpi.GPIO_Speed = GPIO_Speed_50MHz;
	gpi.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LED_GPIO, &gpi);

	GPIO_SetBits(LED_GPIO, LED_BOARD_PIN);
}

void led_board(uint8_t on) {
	GPIO_WriteBit(LED_GPIO, LED_BOARD_PIN, LED_BIT_ACTION(on));
}
