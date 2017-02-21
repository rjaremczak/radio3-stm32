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

void board_init();
void board_initRev1();
void board_initRev2();
bool board_detectRev2();
void board_indicator(bool);
void board_vfoOutBistable(bool, bool);
void board_vfoOut(bool energize);
void board_vfoAtt1(bool energize);
void board_vfoAtt2(bool energize);
void board_vfoAtt3(bool energize);
void board_vnaMode(bool alternate);

#ifdef __cplusplus
}
#endif

#endif
