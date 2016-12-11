/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _RADIO3_H_
#define _RADIO3_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f10x.h>

void radio3_init(void);
void radio3_start(void);

#ifdef __cplusplus
}
#endif

#endif
