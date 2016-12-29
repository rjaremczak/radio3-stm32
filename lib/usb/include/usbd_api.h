#ifndef _USBD_API_H
#define _USBD_API_H 1

#include <usb_def.h>

/** General API **/

enum set_address_t {
  SET_ADDRESS_RESET, SET_ADDRESS_REQUEST, SET_ADDRESS_STATUS
};

void USBDsetDeviceAddress(set_address_t, uint8_t);

/** Library private API **/

void USBDdataReceived(uint8_t, uint32_t);
void USBDcontinueInTransfer(uint8_t);

/** Endpoint configuration API **/

void         USBDdisableAllNonControlEndPoints(void);
UsbResult USBDendPointConfigure(uint8_t, usb_transfer_t, uint16_t, uint16_t);
UsbResult USBDendPointConfigureEx(uint8_t, usb_transfer_t, uint16_t, uint16_t, uint16_t);

/** Endpoint API **/

uint16_t USBDwrite(uint8_t, uint8_t const *, uint16_t);
uint32_t USBDwriteEx(uint8_t, uint8_t const *, uint32_t);
uint16_t USBDread(uint8_t, uint8_t *, uint16_t);
UsbResult USBDgetEndPointStatus(uint8_t, uint16_t *);
UsbResult USBDsetEndPointHalt(uint8_t);
UsbResult USBDclearEndPointHalt(uint8_t);

/** Default control endpoint shortcuts **/

uint16_t USBDwrite0(uint8_t const *, uint16_t);
uint16_t USBDread0(uint8_t *, uint16_t);
void     USBDendPoint0TxVALID(void);
void     USBDendPoint0RxVALID(void);
void     USBDendPoint0TxSTALL(void);
void     USBDendPoint0RxSTALL(void);

/** own functions **/

uint8_t usbd_isWriteInProgress(uint8_t);
void usbd_write_finished(uint8_t);

#endif
