/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _FREQ_METER_H_
#define _FREQ_METER_H_

#include <stdint.h>

void fmeter_init(void);
uint32_t fmeter_read(void);
void fmeter_timebase(void);

#endif
