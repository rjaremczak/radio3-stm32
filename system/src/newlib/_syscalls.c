//
// This file is part of the µOS++ III distribution.
// Parts of this file are from the newlib sources, issued under GPL.
// Copyright (c) 2014 Liviu Ionescu
//

#include <signal.h>

// These functions are defined here to avoid linker errors in freestanding
// applications. They might be called in some error cases from library
// code.
//
// If you detect other functions to be needed, just let us know
// and we'll add them.

int raise(int sig __attribute__((unused))) {
    return -1;
}

int kill(pid_t pid __attribute__((unused)), int sig __attribute__((unused))) {
    return -1;
}

void __attribute__ ((noreturn)) abort(void) {
    abort();
}