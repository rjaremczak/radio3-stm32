/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Functions using Cortex DWT component for time measurement.
 * Experimental, not actually used.
 *
 */

#include <dwt.h>

void dwt_cyclecounter_enable(void) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void dwt_delay_us(uint32_t us) {
	uint32_t delay = us * (SystemCoreClock/1000000);
	DWT->CYCCNT = 0;
	while(DWT->CYCCNT < delay) {};
}

inline uint32_t dwt_cyclecounter(void) {
	return DWT->CYCCNT;
}

inline void dwt_cyclecounter_reset(void) {
	DWT->CYCCNT = 0;
}
