#include <usbd_power.h>

/** USB device without power management **/

int PWRconfigure(__attribute__((unused)) unsigned prio, __attribute__((unused)) unsigned subprio, __attribute__((unused)) int clk) {
  return 0;
}

void PWRmanagementEnable() {
}

void PWRreduce() {
}

void PWRresume() {
}

remote_wakeup_t PWRgetRemoteWakeUp() {
  return RW_DISABLED;
}

void PWRsetRemoteWakeUp(__attribute__((unused)) remote_wakeup_t rw) {
}

void PWRremoteWakeUp() {
}
