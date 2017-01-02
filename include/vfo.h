/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 */

#ifndef _VFO_H_
#define _VFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum class VfoType : uint8_t { NONE, AD9850, AD9851 };

void vfo_init(VfoType vfoType);
void vfo_setFrequency(uint32_t frequency);
uint32_t vfo_frequency();

/*

class Vfo {
public:
    void init(VfoType vfoType) = 0;
    void setFrequency(uint32_t frequency) = 0;
    uint32_t frequency() = 0;
};

*/

#ifdef __cplusplus
}
#endif

#endif
