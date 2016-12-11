/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Implementation of frequency meter based on counting external impulses
 * by redirecting them as TIM-er clock source.
 */

#include <fmeter.h>
#include <stm32f10x.h>

static uint32_t counter = 0;

inline static void reset_timer_counters() {
	TIM_SetCounter(TIM1, 0);
	TIM_SetCounter(TIM2, 0);
}

void fmeter_init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	//GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);

	GPIO_InitTypeDef gpinit;
	gpinit.GPIO_Pin = GPIO_Pin_8;
	gpinit.GPIO_Mode = GPIO_Mode_AF_PP;
	gpinit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpinit);

	TIM_DeInit(TIM1);

	TIM_TimeBaseInitTypeDef tbinit;
	tbinit.TIM_Period = 0xFFFF;
	tbinit.TIM_Prescaler = 0;
	tbinit.TIM_CounterMode = TIM_CounterMode_Up;
	tbinit.TIM_ClockDivision = TIM_CKD_DIV1;
	tbinit.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &tbinit);
	TIM_TIxExternalClockConfig(TIM1, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0);
	TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);

	TIM_DeInit(TIM2);
	tbinit.TIM_Period = 0xFFFF;
	tbinit.TIM_Prescaler = 0;
	tbinit.TIM_CounterMode = TIM_CounterMode_Up;
	tbinit.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &tbinit);
	TIM_ITRxExternalClockConfig(TIM2, TIM_TS_ITR0);

	reset_timer_counters();
	TIM_Cmd(TIM1, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

uint32_t fmeter_read() {
	return counter;
}

void fmeter_timebase() {
	counter = (TIM_GetCounter(TIM2) << 16) | TIM_GetCounter(TIM1);
	TIM_SetCounter(TIM1, 0);
	TIM_SetCounter(TIM2, 0);
}
