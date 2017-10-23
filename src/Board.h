/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include <cstdint>

class Board {
public:
    void preInit();
    void init();
    bool isRevision2();
    void indicator(bool on);
    void vfoOutBistable(bool on1, bool on2);
    void vfoOutMonostable(bool on);
    void att1(bool on);
    void att2(bool on);
    void att3(bool on);
    void amplifier(bool on);
};