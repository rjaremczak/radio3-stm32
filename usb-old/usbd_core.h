#ifndef _USBD_CORE_H
#define _USBD_CORE_H 1

#include "usb_def.h"

/** USB device core system API **/

int USBDcoreConfigure();
void USBDreset(usb_speed_t);
void USBDsuspend();
void USBDwakeup();
void USBDsof(uint16_t);
void USBDtransfer(uint8_t, UsbPid);

#endif
