//
// Created by Robert Jaremczak on 2017.01.11.
//

#pragma once

#include <stm32f10x.h>

class Timer {
public:
    Timer();
    void sleep(uint32_t durationMs) const;
    uint32_t getMillis() const;
    bool isAfter(uint32_t timeMs) const;
    void tickMs();
};