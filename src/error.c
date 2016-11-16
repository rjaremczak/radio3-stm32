//
// Created by Robert Jaremczak on 2016.11.16.
//

#include "error.h"
#include "delay.h"

static const unsigned delayTime = 3000000U;

void Error(int n) {
    int i;

    board_led_yellow(0);
    for (i = 0; i < n; ++i) {
        Delay(delayTime);
        board_led_yellow(1);
        Delay(delayTime);
        board_led_yellow(0);
    }
    Delay(9U * delayTime);
}

