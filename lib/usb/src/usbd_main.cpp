
#include <delay.h>
#include <irq.h>
#include <usbd_configure.h>

void usbd_init(void) {
    delay(2000000);
    IRQprotectionConfigure();
    USBDconfigure(MIDDLE_IRQ_PRIO, 0, 72);
}