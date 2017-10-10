//
// Created by Robert Jaremczak on 2017.10.10.
//

#include <cstdarg>
#include <cstdio>
#include "log.h"

/** Defines for Cortex-M debug unit **/

#define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000)    // Stimulus Port Register word acces
#define ITM_STIM_U8  (*(volatile         char*)0xE0000000)    // Stimulus Port Register byte acces
#define ITM_ENA      (*(volatile unsigned int*)0xE0000E00)    // Trace Enable Ports Register
#define ITM_TCR      (*(volatile unsigned int*)0xE0000E80)    // Trace control register

static Timer *timer = nullptr;
static bool enabled = false;

static void swo_print(char c) {
    if ((ITM_TCR & 1) == 0) return;
    if ((ITM_ENA & 1) == 0) return;

    while ((ITM_STIM_U8 & 1) == 0) {}
    ITM_STIM_U8 = c;
}

static void swo_print(const char *str) {
    while (*str) { swo_print(*str++); }
}

static void swo_printf(const char *fmt, ...) {
    static char buf[100];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);
    swo_print(buf);
}

void log_init(Timer *ptr) {
    timer = ptr;
    enabled = true;
}

void log(const char *fmt, ...) {
    if(!enabled) return;

    static char buf[100];
    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);
    if(timer) swo_printf("%010d ", timer->getMillis());
    swo_print(buf);
    swo_print('\n');
}