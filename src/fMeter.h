/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _FREQ_METER_H_
#define _FREQ_METER_H_

#include <cstdint>

class FMeter {
    uint32_t timebaseCounter = 1000;
    uint32_t counter = 0;
public:
    void init();
    uint32_t read();
    void tickMs();
};

#endif
