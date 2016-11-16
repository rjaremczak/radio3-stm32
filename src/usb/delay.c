#include <delay.h>

void Delay(volatile unsigned count) {
  while (count--);
}
