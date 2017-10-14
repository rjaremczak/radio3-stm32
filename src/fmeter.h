/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _FREQ_METER_H_
#define _FREQ_METER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void fmeter_init();
uint32_t fmeter_read();
void fmeter_timebase();

#ifdef __cplusplus
}
#endif

#endif
