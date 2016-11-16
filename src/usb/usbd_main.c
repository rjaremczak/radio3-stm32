#include <board_init.h>
#include <boot.h>
#include <delay.h>
#include <error.h>
#include <irq.h>
#include <usbd_configure.h>
#include <usbd_power.h>

void usbd_main(void) {
    int sysclk;
    usb_speed_t speed;
    usb_phy_t phy;

    GetBootParams(&sysclk, &speed, &phy);
    //AllPinsDisable();

    //LEDconfigure();
    //RedLEDon();
    /* USB should be configured just after power on. In the other case
       the pull-up resistor must be kept off until configuration is not
       finished. */
    //ErrorResetable(USBDpreConfigure(speed, phy), 1);

    //USBDpreConfigure(speed, phy); -- disable pull-up resistor

    /* Wait about 0.5 to 2 s for JTAG before you configure the clock.
       Simulation of a long-term operation, which makes necessary
       to divide USB configuration into two stages. */

    Delay(2000000);
    IRQprotectionConfigure();
    //ErrorResetable(ClockConfigure(sysclk), 2); --- this is done in startup code

    //ErrorResetable(LCDconfigure(), 3);
    //ErrorResetable(PWRconfigure(HIGH_IRQ_PRIO, 0, sysclk), 4);
    PWRconfigure(HIGH_IRQ_PRIO, 0, sysclk);

    //ErrorResetable(USBDconfigure(MIDDLE_IRQ_PRIO, 0, sysclk), 5);
    USBDconfigure(MIDDLE_IRQ_PRIO, 0, sysclk);

    //RedLEDoff();
    /* Configuration process is finished successfully. */
    //for (;;) LCDrunRefresh();
}
