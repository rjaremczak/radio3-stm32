#include "delay.h"

void delayUs(unsigned us) {
  volatile unsigned counter = 8*us;
  while(counter--);
}