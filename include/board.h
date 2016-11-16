/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _BOARD_H_
#define _BOARD_H_

void board_init(void);
void board_integrated_led(uint8_t);
void board_led_green(uint8_t);
void board_led_yellow(uint8_t);
void board_buzzer(uint8_t);

#endif
