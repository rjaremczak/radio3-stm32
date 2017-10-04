#include <swo.h>
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "UsbVCom.h"

void main2(void) {

    SWO_printf("started\n");

    Set_System();
    Set_USBClock();
    //USB_Interrupts_Config();
    //USB_Init();

    while (1) {
    }
}