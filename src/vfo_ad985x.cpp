/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * control library for AD9850/1 DDS chips
 *
 */

#include "vfo.h"

#include <stm32f10x.h>

// all pins must be in the same port

static const auto GPIO_PORT = GPIOB;
static const auto PIN_DATA = GPIO_Pin_9;
static const auto PIN_WCLK = GPIO_Pin_8;
static const auto PIN_FQUD = GPIO_Pin_7;
static const auto PIN_RESET = GPIO_Pin_6;

static unsigned XTAL;
static uint8_t CONTROL_W0;

static uint32_t calcFreq(uint32_t hz) {
    return (uint32_t)((hz * 4294967296U) / XTAL);
}

static volatile uint32_t current_frequency = 0;

static void wait() {
	volatile int i=4;
	while(i) { i--; }
}

static void pulse(uint16_t pin) {
	GPIO_WriteBit(GPIO_PORT, pin, Bit_SET);
	wait();
	GPIO_WriteBit(GPIO_PORT, pin, Bit_RESET);
	wait();
}

static void send_byte(uint8_t data) {
	for(int i=0; i<8; i++) {
		GPIO_WriteBit(GPIO_PORT, PIN_DATA, static_cast<BitAction>(data & 0x01));
		pulse(PIN_WCLK);
		data >>= 1;
	}
}

void vfo_setFrequency(uint32_t frequency) {
    uint32_t value = calcFreq(frequency);
	for(int i=0; i<4; i++) {
		send_byte((uint8_t) (value & 0xff));
		value >>= 8;
	}
	send_byte(CONTROL_W0);
	pulse(PIN_FQUD);
	current_frequency = frequency;
}

uint32_t vfo_frequency() {
	return current_frequency;
}

void vfo_init(VfoType vfoType) {
    switch (vfoType) {
        case VfoType::AD9850 :
            XTAL = 125000000U;
            CONTROL_W0 = 0x00;
            break;
        case VfoType::AD9851 :
            XTAL = 180000000U;
            CONTROL_W0 = 0x01;
            break;
        default: return;
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef gpiod;
	gpiod.GPIO_Pin = PIN_DATA | PIN_WCLK | PIN_FQUD | PIN_RESET;
	gpiod.GPIO_Speed = GPIO_Speed_50MHz;
	gpiod.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_PORT, &gpiod);
	GPIO_ResetBits(GPIO_PORT, PIN_DATA | PIN_WCLK | PIN_FQUD | PIN_RESET);

	// reset and enable serial operations

	pulse(PIN_RESET);
	pulse(PIN_WCLK);
	pulse(PIN_FQUD);
}
