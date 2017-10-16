//
// Created by Robert Jaremczak on 2017.10.10.
//

#ifndef RADIO3_STM32_LOGGER_H
#define RADIO3_STM32_LOGGER_H


#include <Timer.h>

void log_init(Timer *ptr);
void log(const char *fmt, ...);

#endif //RADIO3_STM32_LOGGER_H
