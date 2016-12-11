/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * control library for AD9850/1 DDS chips
 *
 */

#include <ad985x.h>

// define AD9850 or AD9851

#define DDS_AD9851

// all pins must be in the same port

#define GPIO_PORT 		GPIOB
#define PIN_DATA		GPIO_Pin_9
#define PIN_WCLK		GPIO_Pin_8
#define PIN_FQUD		GPIO_Pin_7
#define PIN_RESET		GPIO_Pin_6

#ifdef DDS_AD9850
static const auto XTAL = 125000000U;
static const auto CONTROL_W0 = 0x00;
#endif

#ifdef DDS_AD9851
static const auto XTAL = 180000000U;
static const auto CONTROL_W0 = 0x01;
#endif

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

void ad985x_setFrequency(uint32_t frequency) {
    uint32_t value = calcFreq(frequency);
	for(int i=0; i<4; i++) {
		send_byte((uint8_t) (value & 0xff));
		value >>= 8;
	}
	send_byte(CONTROL_W0);
	pulse(PIN_FQUD);
	current_frequency = frequency;
}

uint32_t ad985x_frequency() {
	return current_frequency;
}

void ad985x_init() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef initdata;
	initdata.GPIO_Pin = PIN_DATA|PIN_WCLK|PIN_FQUD|PIN_RESET;
	initdata.GPIO_Speed = GPIO_Speed_50MHz;
	initdata.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_PORT, &initdata);
	GPIO_ResetBits(GPIO_PORT, PIN_DATA|PIN_WCLK|PIN_FQUD|PIN_RESET);

	// reset and enable serial operations

	pulse(PIN_RESET);
	pulse(PIN_WCLK);
	pulse(PIN_FQUD);
}
