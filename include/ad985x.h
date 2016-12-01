/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _AD985X_H_
#define _AD985X_H_

#include <stm32f10x.h>

void ad985x_init(void);
void ad985x_setFrequency(uint32_t);
uint32_t ad985x_frequency(void);

#endif
