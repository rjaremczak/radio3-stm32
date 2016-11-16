#ifndef _USBD_POWER_H
#define _USBD_POWER_H 1

/** USB device power management API **/

typedef enum {
  RW_DISABLED = 0, RW_ENABLED
} remote_wakeup_t;

int PWRconfigure(unsigned, unsigned, int);
void PWRmanagementEnable(void);
void PWRreduce(void);
void PWRresume(void);
remote_wakeup_t PWRgetRemoteWakeUp(void);
void PWRsetRemoteWakeUp(remote_wakeup_t);
void PWRremoteWakeUp(void);

#endif
