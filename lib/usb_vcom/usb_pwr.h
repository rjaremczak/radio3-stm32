#ifndef __USB_PWR_H
#define __USB_PWR_H

enum RESUME_STATE {
    RESUME_EXTERNAL,
    RESUME_INTERNAL,
    RESUME_LATER,
    RESUME_WAIT,
    RESUME_START,
    RESUME_ON,
    RESUME_OFF,
    RESUME_ESOF
};

enum DEVICE_STATE {
    UNCONNECTED,
    ATTACHED,
    POWERED,
    SUSPENDED,
    ADDRESSED,
    CONFIGURED
};

void Suspend();
void Resume_Init();
void Resume(RESUME_STATE eResumeSetVal);
RESULT PowerOn();
RESULT PowerOff();

/* External variables --------------------------------------------------------*/
extern __IO uint32_t bDeviceState; /* USB device status */
extern __IO bool fSuspendEnabled;  /* true when suspend is possible */

#endif  /*__USB_PWR_H*/
