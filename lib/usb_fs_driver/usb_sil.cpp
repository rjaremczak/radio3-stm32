#include "usb_lib.h"

/**
  * Function Name  : USB_SIL_Init
  * Description    : Initialize the USB Device IP and the Endpoint 0.
  * Input          : None.
  * Output         : None.
  * Return         : Status.
  **/
uint32_t USB_SIL_Init()
{
  /* USB interrupts initialization */
  /* clear pending interrupts */
  _SetISTR(0);
  wInterrupt_Mask = IMR_MSK;
  /* set interrupts mask */
  _SetCNTR(wInterrupt_Mask);
  return 0;
}