#ifndef _USBD_CONFIGURE_H
#define _USBD_CONFIGURE_H 1

#include "usb_def.h"

/** USB device initialization API **/

int USBDpreConfigure(usb_speed_t, usb_phy_t);
int USBDconfigure(unsigned, unsigned, int);

#endif
