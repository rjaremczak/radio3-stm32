//
// Created by Robert Jaremczak on 2017.01.11.
//

#include "Timer.h"

static volatile uint32_t millis = 0;

Timer::Timer() {
    SysTick_Config(SystemCoreClock / 1000);
}

void Timer::sleep(uint32_t durationMs) const {
    auto ms = millis + durationMs + 1;
    while(millis < ms) {};
}

uint32_t Timer::getMillis() const {
    return millis;
}

bool Timer::isAfter(uint32_t timeMs) const {
    return millis > timeMs;
}

void Timer::tick(uint32_t msPerTick) {
    millis += msPerTick;
}

void Timer::tickMs() {
    millis++;
}

