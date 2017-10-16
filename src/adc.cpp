/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * ADC library
 *
 */

#include <stm32f10x.h>
#include "adc.h"
#include "delay.h"

void adc_init() {
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef gpi;
	gpi.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	gpi.GPIO_Mode = GPIO_Mode_AIN;
	gpi.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpi);

	ADC_InitTypeDef adci;
	adci.ADC_Mode = ADC_Mode_Independent;
	adci.ADC_ScanConvMode = DISABLE;
	adci.ADC_ContinuousConvMode = DISABLE;
	adci.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	adci.ADC_DataAlign = ADC_DataAlign_Right;
	adci.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &adci);
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1)) {};

	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1)) {};
}

static uint16_t adc_readOnce(uint8_t channel) {
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {}
	return ADC_GetConversionValue(ADC1);
}

static uint16_t adc_read(uint8_t channel, uint8_t avgSamples) {
	uint32_t acc = 0;
	for(char i=0; i<avgSamples; i++) {
		acc += adc_readOnce(channel);
		delayUs(5);
	}

	return (uint16_t) (acc / avgSamples);
}

uint16_t adc_readLogarithmicProbe(uint8_t avgSamples) {
	return adc_read(ADC_Channel_0, avgSamples);
}

uint16_t adc_readLinearProbe(uint8_t avgSamples) {
	return adc_read(ADC_Channel_1, avgSamples);
}

uint16_t adc_readVnaGainValue(uint8_t avgSamples) {
	return adc_read(ADC_Channel_2, avgSamples);
}

uint16_t adc_readVnaPhaseValue(uint8_t avgSamples) {
	return adc_read(ADC_Channel_3, avgSamples);
}
