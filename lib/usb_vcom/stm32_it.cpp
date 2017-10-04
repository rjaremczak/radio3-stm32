#include "hw_config.h"
#include "usb_istr.h"

/*******************************************************************************
* Function Name  : USB_IRQHandler
* Description    : This function handles USB Low Priority interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern "C" void USB_LP_CAN1_RX0_IRQHandler(void) {
    USB_Istr();
}
/*******************************************************************************
* Function Name  : USB_FS_WKUP_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern "C" void USBWakeUp_IRQHandler(void) {
    EXTI_ClearITPendingBit(EXTI_Line18);
}