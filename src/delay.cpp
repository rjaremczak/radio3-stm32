#include "delay.h"

void delay(volatile unsigned count) {
  while (count--);
}

void delayUs(unsigned us) {
  volatile unsigned counter = 8*us;
  while(counter--);
}