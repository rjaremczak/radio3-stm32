//
// This file is part of the µOS++ III distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

// This module contains the startup code for a portable embedded
// C/C++ application, built with newlib.
//
// Control reaches here from the reset handler via jump or call.
//
// The actual steps performed by _start are:
// - copy the initialised data region(s)
// - clear the BSS region(s)
// - initialise the system
// - run the preinit/init array (for the C++ static constructors)
// - initialise the arc/argv
// - branch to main()
// - run the fini array (for the C++ static destructors)
// - call _exit(), directly or via exit()
//
// If OS_INCLUDE_STARTUP_INIT_MULTIPLE_RAM_SECTIONS is defined, the
// code is capable of initialising multiple regions.
//
// The normal configuration is standalone, with all support
// functions implemented locally.
//
// For this to be called, the project linker must be configured without
// the startup sequence (-nostartfiles).

// ----------------------------------------------------------------------------

#include <stdint.h>
#include <sys/types.h>
#include <errno.h>
#include "stm32f10x.h"

// ----------------------------------------------------------------------------

extern unsigned int __vectors_start;
extern unsigned int _sidata;
extern unsigned int _sdata;
extern unsigned int _edata;
extern unsigned int __bss_start__;
extern unsigned int __bss_end__;

extern void main(void);

// ----------------------------------------------------------------------------

inline static void __initialize_data(unsigned int *from, unsigned int *region_begin, unsigned int *region_end) {
    // Iterate and copy word by word.
    // It is assumed that the pointers are word aligned.
    unsigned int *p = region_begin;
    while (p < region_end) { *p++ = *from++; }
}

inline static void __initialize_bss(unsigned int *region_begin, unsigned int *region_end) {
    // Iterate and clear word by word.
    // It is assumed that the pointers are word aligned.
    unsigned int *p = region_begin;
    while (p < region_end)
        *p++ = 0;
}

// These magic symbols are provided by the linker.
extern void (*__preinit_array_start[])(void) __attribute__((weak));
extern void (*__preinit_array_end[])(void) __attribute__((weak));
extern void (*__init_array_start[])(void) __attribute__((weak));
extern void (*__init_array_end[])(void) __attribute__((weak));
extern void (*__fini_array_start[])(void) __attribute__((weak));
extern void (*__fini_array_end[])(void) __attribute__((weak));

// Iterate over all the preinit/init routines (mainly static constructors).
inline static void __run_init_array(void) {
    int count;
    int i;

    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start[i]();

    // If you need to run the code in the .init section, please use
    // the startup files, since this requires the code in crti.o and crtn.o
    // to add the function prologue/epilogue.
    //_init(); // DO NOT ENABE THIS!

    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++)
        __init_array_start[i]();
}

// This is the place where Cortex-M core will go immediately after reset,
// via a call or jump from the Reset_Handler.
//
// For the call to work, and for the call to __initialize_hardware_early()
// to work, the reset stack must point to a valid internal RAM area.

// -----------------------------------------------------------------------------
// The Release version is optimised to a quick branch to _start.
// -----------------------------------------------------------------------------

void _start(void) {
    // Call the CSMSIS system initialisation routine.
    SystemInit();

    // Set VTOR to the actual address, provided by the linker script.
    // Override the manual, possibly wrong, SystemInit() setting.
    SCB->VTOR = (uint32_t) (&__vectors_start);

    // Use Old Style DATA and BSS section initialisation,
    // that will manage a single BSS sections.

    __initialize_data(&_sidata, &_sdata, &_edata);
    __initialize_bss(&__bss_start__, &__bss_end__);

    // Call the CSMSIS system clock routine to store the clock frequency
    // in the SystemCoreClock global RAM location.
    SystemCoreClockUpdate();

    // Call the standard library initialisation (mandatory for C++ to
    // execute the constructors for the static objects).
    __run_init_array();

    // Call the main entry point, and save the exit code.
    main();
}
