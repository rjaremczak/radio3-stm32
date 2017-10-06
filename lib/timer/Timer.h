//
// Created by Robert Jaremczak on 2017.01.11.
//

#ifndef STM32F103_MD_TEMPLATE_TIMER_H
#define STM32F103_MD_TEMPLATE_TIMER_H

#include <stm32f10x.h>

class Timer {
public:
    Timer();
    void tick();
    void sleep(uint32_t durationMs) const;
    uint32_t getMillis() const;
    bool isAfter(uint32_t timeMs) const;
};

#endif //STM32F103_MD_TEMPLATE_TIMER_H