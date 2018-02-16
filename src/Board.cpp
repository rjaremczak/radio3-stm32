/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * LED control functions.
 *
 */

#include <stm32f10x.h>
#include "Board.h"

void Board::detectHardwareRevision() {
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

    gpi.GPIO_Speed = GPIO_Speed_50MHz;
    gpi.GPIO_Mode = GPIO_Mode_IPU;
    gpi.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &gpi);

    if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
        hardwareRevision = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) ? HardwareRevision::rev2 : HardwareRevision::rev3;
    } else {
        hardwareRevision = HardwareRevision::rev1;
    }
}

void Board::init() {
    detectHardwareRevision();

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

void Board::indicator(bool on) {
    BitAction val = toBitAction(!on);
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, val);
    GPIO_WriteBit(GPIOB, GPIO_Pin_13, val);
}

void Board::vfoOutBistable(bool on1, bool on2) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, toBitAction(on1));
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, toBitAction(on2));
}