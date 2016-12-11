#include <delay.h>

void delay(volatile unsigned count) {
  while (count--);
}
