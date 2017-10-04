//
// Created by Robert Jaremczak on 2017.10.04.
//

#include <usb_lib.h>
#include <swo.h>
#include "UsbVCom.h"

UsbVCom::UsbVCom(Timer &timer) : timer(timer) {

}

void UsbVCom::init() {
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
}

void UsbVCom::ep3out() {
    uint16_t count = GetEPRxCount(EP3_OUT & 0x7F);
    PMAToUserBufferCopy(rxBuf, GetEPRxAddr(EP3_OUT & 0x7F), count);
    SWO_printf("received %d bytes\n", count);
    UserToPMABufferCopy(rxBuf, ENDP1_TXADDR, count);
    SetEPTxCount(ENDP1, count);
    SetEPTxValid(ENDP1);
}

void UsbVCom::ep1in() {
    SetEPRxValid(ENDP3);
}

void UsbVCom::sof() {

}
