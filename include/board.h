/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _BOARD_H_
#define _BOARD_H_

void board_init(void);
void board_ledOnModule(uint8_t);
void board_ledGreen(uint8_t);
void board_ledYellow(uint8_t);
void board_buzzer(uint8_t);

#endif
