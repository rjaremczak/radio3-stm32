
#include <delay.h>
#include <irq.h>
#include <usbd_configure.h>

void usbd_main(void) {
    int sysclk = 72;
    Delay(2000000);
    IRQprotectionConfigure();
    USBDconfigure(MIDDLE_IRQ_PRIO, 0, sysclk);
}