/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _ADC_H_
#define _ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void adc_init();
uint16_t adc_readLogarithmicProbe(uint8_t avgSamples);
uint16_t adc_readLinearProbe(uint8_t avgSamples);
uint16_t adc_readVnaGainValue(uint8_t avgSamples);
uint16_t adc_readVnaPhaseValue(uint8_t avgSamples);

#ifdef __cplusplus
}
#endif

#endif
