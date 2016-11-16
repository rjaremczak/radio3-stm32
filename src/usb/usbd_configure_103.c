#include <board_usb_def.h>
#include <usb_regs.h>
#include <usbd_configure.h>
#include <usbd_core.h>

/** Low level USB device initialization for STM32F102 and STM32F103 **/

/* USB core needs 48 MHz clock.
    clk - SYSCLK clock in MHz */
static int USBDclockConfigure(int clk) {
  if (clk == 48)
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);
  else if (clk == 72)
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  else
    return -1;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  return 0;
}

/* Configure interrupt controler.
    prio    - preemption priority
    subprio - service priority when the same prio */
static void USBDinterruptConfigure(unsigned prio, unsigned subprio) {
  NVIC_InitTypeDef NVIC_InitStruct;

  NVIC_InitStruct.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = prio;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = subprio;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

/* Switch USB hardware on and configure interrupt mask. */
static void USBDperipheralConfigure(void) {
  /* Disable all interrupts and reset USB device. */
  _SetCNTR(CNTR_FRES);
  /* Bits PDWN and FRES are set after microcontroller reset - the CNTR
     register is initialized with the value (CNTR_PDWN | CNTR_FRES) ==
     0x0003. The device is in the reset state as long as the CNTR_FRES
     bit is set. It is necessary to wait t_STARTUP time, before
     clearing the PDWN and FRES bits. Assume, that this time is
     elapsed, e.g., during clock and PLLs initialization. */
  _SetCNTR(0);
  /* Clear pending interrupts. */
  _SetISTR(0);
  /* Set disired interrupt mask. */
  _SetCNTR(CNTR_CTRM | CNTR_RESETM | CNTR_SOFM);
  /* Simulate cable plug in. */
  //PullUpResistor(1);
}

/** USB device initialization API **/

/* Call the USBpreConfigure function as early as possible after
   microcontroller reset. The main configuration is provided later
   in the USBconfigure function. Phy transceiver is integrated. */

/*
int USBDpreConfigure(usb_speed_t speed, __attribute__((unused)) usb_phy_t phy) {
  return PullUpConfigure(speed);
}
 */

/* Configure USB interface.
    prio    - preemption priority
    subprio - service priority when the same prio
    clk     - SYSCLK clock in MHz */
int USBDconfigure(unsigned prio, unsigned subprio, int clk) {
  if (USBDcoreConfigure() < 0)
    return -1;
  if (USBDclockConfigure(clk) < 0)
    return -1;
  USBDinterruptConfigure(prio, subprio);
  USBDperipheralConfigure();
  return 0;
}
