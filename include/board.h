/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void board_init(void);
void board_ledOnModule(uint8_t);
void board_ledGreen(uint8_t);
void board_ledYellow(uint8_t);
void board_vfoOutBistable(uint8_t, uint8_t);
void board_indicateError(uint8_t);
void board_vfoOut(bool energize);
void board_vfoAtt_1(bool energize);
void board_vfoAtt_2(bool energize);
void board_vfoAtt_3(bool energize);

#ifdef __cplusplus
}
#endif

#endif
