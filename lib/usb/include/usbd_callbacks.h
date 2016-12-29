#ifndef _USBD_CALLBACKS_H
#define _USBD_CALLBACKS_H 1

#include <usb_def.h>

/** USB device application callbacks **/

struct UsbdCallbackList {
    /* Application configuration */
    int (*Configure)(void);

    /* Interrupt callbacks */
    uint8_t (*Reset)(usb_speed_t speed);
    void (*SoF)(uint16_t frameNumber);

    /* Standard request callbacks */
    UsbResult (*GetDescriptor)(uint16_t wValue, uint16_t wIndex, uint8_t const **data, uint16_t *length);
    UsbResult (*SetDescriptor)(uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t **data);
    uint8_t (*GetConfiguration)(void);
    UsbResult (*SetConfiguration)(uint16_t configurationValue);
    uint16_t (*GetStatus)(void);
    UsbResult (*GetInterface)(uint16_t interface, uint8_t *setting);
    UsbResult (*SetInterface)(uint16_t interface, uint16_t setting);
    UsbResult (*ClearDeviceFeature)(uint16_t featureSelector);
    UsbResult (*SetDeviceFeature)(uint16_t featureSelector);

    /* Class request callbacks */
    UsbResult (*ClassNoDataSetup)(UsbSetupPacket const *setup);
    UsbResult (*ClassInDataSetup)(UsbSetupPacket const *setup, uint8_t const **data, uint16_t *length);
    UsbResult (*ClassOutDataSetup)(UsbSetupPacket const *setup, uint8_t **data);
    void (*ClassStatusIn)(UsbSetupPacket const *setup);

    /* Endpoint callbacks, the callback index is the endpoint address decreased by 1. */
    void (*EPin[15])(void);
    void (*EPout[15])(void);

    /* Peripheral power management callbacks */
    void (*Suspend)(void);
    void (*Wakeup)(void);
};

/** Application initialization callback **/

UsbdCallbackList const *USBDgetApplicationCallbacks(void);

#endif
