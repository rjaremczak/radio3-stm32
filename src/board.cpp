/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * LED control functions.
 *
 */

#include <stm32f10x.h>
#include "board.h"

inline static BitAction toBitAction(bool b) { return b ? Bit_SET : Bit_RESET; }

void board_preInit() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef gpi;
    gpi.GPIO_Speed = GPIO_Speed_50MHz;
    gpi.GPIO_Mode = GPIO_Mode_Out_OD;
    gpi.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &gpi);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpi.GPIO_Mode = GPIO_Mode_Out_OD;
    gpi.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOB, &gpi);
    GPIO_SetBits(GPIOB, gpi.GPIO_Pin);
}

bool board_isRevision2() {
    GPIO_InitTypeDef gpi;
    gpi.GPIO_Speed = GPIO_Speed_50MHz;
    gpi.GPIO_Mode = GPIO_Mode_IPU;
    gpi.GPIO_Pin = GPIO_Pin_12;
    GPIO_Init(GPIOB, &gpi);
    return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
}

void board_init() {
    GPIO_InitTypeDef gpi;
    gpi.GPIO_Speed = GPIO_Speed_50MHz;
    gpi.GPIO_Mode = GPIO_Mode_Out_PP;
    gpi.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &gpi);
    GPIO_ResetBits(GPIOB, gpi.GPIO_Pin);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpi.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOA, &gpi);
    GPIO_ResetBits(GPIOA, GPIO_Pin_13);

}

void board_indicator(bool on) {
    BitAction val = toBitAction(!on);
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, val);
    GPIO_WriteBit(GPIOB, GPIO_Pin_13, val);
}

void board_vfoOutBistable(bool pin1, bool pin10) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, toBitAction(pin1));
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, toBitAction(pin10));
}

void board_vfoOut(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, toBitAction(energize));
}

void board_vfoAtt1(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, toBitAction(energize));
}

void board_vfoAtt2(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_10, toBitAction(energize));
}

void board_vfoAtt3(bool energize) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_11, toBitAction(energize));
}

void board_vfoAmplifier(bool enable) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_14, toBitAction(enable));
}