#ifndef _DELAY_H
#define _DELAY_H 1

#ifdef __cplusplus
extern "C" {
#endif

void delay(volatile unsigned);
void delayUs(unsigned us);

#ifdef __cplusplus
}
#endif

#endif
