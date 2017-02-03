/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * LED control functions.
 *
 */

#include <stm32f10x.h>
#include <board.h>
#include <delay.h>

#define LED_BIT_ACTION(on)	(on ? Bit_RESET : Bit_SET)

inline static BitAction toBitAction(bool b) { return b ? Bit_SET : Bit_RESET; }

void board_init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef gpi;
	gpi.GPIO_Pin = GPIO_Pin_13;
	gpi.GPIO_Speed = GPIO_Speed_50MHz;
	gpi.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOC, &gpi);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpi.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_Init(GPIOB, &gpi);
    GPIO_SetBits(GPIOB, gpi.GPIO_Pin);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpi.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15;
    gpi.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &gpi);
    GPIO_ResetBits(GPIOB, gpi.GPIO_Pin);
}

void board_ledOnModule(uint8_t on) {
	GPIO_WriteBit(GPIOC, GPIO_Pin_13, LED_BIT_ACTION(on));
}

void board_ledGreen(uint8_t on) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_13, LED_BIT_ACTION(on));
}

void board_ledYellow(uint8_t on) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_14, LED_BIT_ACTION(on));
}

void board_vfoOutBistable(uint8_t pin1, uint8_t pin10) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, LED_BIT_ACTION(pin1));
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, LED_BIT_ACTION(pin10));
}

void board_vfoOut(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, toBitAction(energize));
}

void board_vfoAtt_1(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, toBitAction(energize));
}

void board_vfoAtt_2(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_10, toBitAction(energize));
}

void board_vfoAtt_3(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_11, toBitAction(energize));
}

void board_indicateError(uint8_t times) {
    static const unsigned delayTime = 3000000U;

    int i;

    board_ledYellow(0);
    for (i = 0; i < times; ++i) {
        delay(delayTime);
        board_ledYellow(1);
        delay(delayTime);
        board_ledYellow(0);
    }
    delay(9U * delayTime);
}
