/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _ADC_H_
#define _ADC_H_

void adc_init(void);
uint16_t adc_readLogarithmicProbe(void);
uint16_t adc_readLinearProbe(void);
uint16_t adc_readVnaGainValue(void);
uint16_t adc_readVnaPhaseValue(void);

#endif
