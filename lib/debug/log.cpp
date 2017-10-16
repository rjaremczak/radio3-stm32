//
// Created by Robert Jaremczak on 2017.10.10.
//

#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include "log.h"

namespace {
    Timer *timer = nullptr;
    bool enabled = false;

    void swo_print(const char *str) {
        while (*str) { ITM_SendChar(*str++); }
    }
}

void log_init(Timer *ptr) {
    timer = ptr;
    enabled = true;
}

void log(const char *fmt, ...) {
    if (!enabled) return;

    static char buf[100];
    snprintf(buf, sizeof(buf), "%010lu ", timer->getMillis());

    auto offset = strlen(buf);
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf + offset, sizeof(buf) - offset - 1, fmt, va);
    va_end(va);

    offset = strlen(buf);
    buf[offset++] = '\n';
    buf[offset] = 0;
    swo_print(buf);
}