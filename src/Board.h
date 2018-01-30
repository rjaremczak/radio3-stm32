/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include <cstdint>

enum class HardwareRevision : uint8_t {
    AUTODETECT, VERSION_1, VERSION_2, VERSION_3
};

class Board {
public:
    void preInit();
    void init();
    HardwareRevision detectHardwareRevision();
    void indicator(bool on);
    void vfoOutBistable(bool on1, bool on2);
    void vfoOutMonostable(bool on);
    void att1(bool on);
    void att2(bool on);
    void att3(bool on);
    void amplifier(bool on);
};