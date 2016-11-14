/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _DWT_H_
#define _DWT_H_

#include <stm32f10x.h>

void dwt_cyclecounter_enable(void);
void dwt_delay_us(uint32_t);
uint32_t dwt_cyclecounter(void);
void dwt_cyclecounter_reset(void);

#endif
