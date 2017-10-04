#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include "platform_config.h"

void Set_System(void);
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void Get_SerialNum(void);
void VCP_SendRxBufPacketToUsb(void);
void VCP_Data_InISR(void);

#endif  /*__HW_CONFIG_H*/