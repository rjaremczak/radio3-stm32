/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#pragma once

#include <cstdint>

namespace {
    BitAction toBitAction(bool b) { return b ? Bit_SET : Bit_RESET; }
}

enum class HardwareRevision : uint8_t {
    unknown, rev1, rev2, rev3
};

class Board {
    HardwareRevision hardwareRevision;
    void detectHardwareRevision();

public:
    void init();
    void indicator(bool on);
    void vfoOutBistable(bool on1, bool on2);

    inline HardwareRevision getHardwareRevision() { return hardwareRevision; };
    inline void att1(bool energize) { GPIO_WriteBit(GPIOB, GPIO_Pin_12, toBitAction(energize)); }
    inline void att2(bool energize) { GPIO_WriteBit(GPIOB, GPIO_Pin_10, toBitAction(energize)); }
    inline void att3(bool energize) { GPIO_WriteBit(GPIOB, GPIO_Pin_11, toBitAction(energize)); }
    inline void amplifier(bool enable) { GPIO_WriteBit(GPIOB, GPIO_Pin_14, toBitAction(enable)); }
    inline void vfoOutMonostable(bool on) { GPIO_WriteBit(GPIOB, GPIO_Pin_15, toBitAction(on)); }
};

