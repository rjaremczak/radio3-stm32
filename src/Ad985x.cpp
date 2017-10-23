/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * control library for AD9850/1 DDS chips
 *
 */

#include "Vfo.h"
#include "delay.h"
#include "Ad985x.h"
#include <stm32f10x.h>

// all pins must be in the same port

namespace {
    const auto GPIO_PORT = GPIOB;
    const auto PIN_DATA = GPIO_Pin_9;
    const auto PIN_WCLK = GPIO_Pin_8;
    const auto PIN_FQUD = GPIO_Pin_7;
    const auto PIN_RESET = GPIO_Pin_6;
}

void Ad985x::sendByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        GPIO_WriteBit(GPIO_PORT, PIN_DATA, static_cast<BitAction>(data & 0x01));
        pulse(PIN_WCLK);
        data >>= 1;
    }
}

void Ad985x::init(Vfo::Type type) {
    switch (type) {
        case Vfo::Type::DDS_AD9850 :
            XTAL = 125000000U;
            CONTROL_W0 = 0x00;
            break;
        case Vfo::Type::DDS_AD9851 :
            XTAL = 180000000U;
            CONTROL_W0 = 0x01;
            break;
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpiod;
    gpiod.GPIO_Pin = PIN_DATA | PIN_WCLK | PIN_FQUD | PIN_RESET;
    gpiod.GPIO_Speed = GPIO_Speed_50MHz;
    gpiod.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_PORT, &gpiod);
    GPIO_ResetBits(GPIO_PORT, PIN_DATA | PIN_WCLK | PIN_FQUD | PIN_RESET);

    pulse(PIN_RESET);
    pulse(PIN_WCLK);
    pulse(PIN_FQUD);
}

uint32_t Ad985x::calcFreq(uint32_t hz) {
    return (uint32_t) ((hz * 4294967296U) / XTAL);
}

void Ad985x::pulse(uint16_t pin) {
    GPIO_WriteBit(GPIO_PORT, pin, Bit_SET);
    delayUs(1);
    GPIO_WriteBit(GPIO_PORT, pin, Bit_RESET);
    delayUs(1);
}

void Ad985x::setFrequency(uint32_t frequency) {
    currentFrequency = frequency;
    uint32_t value = calcFreq(frequency);
    for (int i = 0; i < 4; i++) {
        sendByte((uint8_t) (value & 0xff));
        value >>= 8;
    }
    sendByte(CONTROL_W0);
    pulse(PIN_FQUD);
}

uint32_t Ad985x::frequency() {
    return currentFrequency;
}
