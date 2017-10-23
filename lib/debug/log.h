//
// Created by Robert Jaremczak on 2017.10.10.
//

#pragma once

#include <Timer.h>

void log_init(Timer *ptr);
void log(const char *fmt, ...);