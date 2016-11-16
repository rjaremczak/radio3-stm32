#ifndef _BOARD_DEF_H
#define _BOARD_DEF_H 1

#include <misc.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_flash.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_pwr.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <xcat.h>

/** Gadget **/

/* Wszystkie stałe powinny być definiowane w plikach nagłówkowych,
   ale Standard Peripheral Library na to nie pozwala... */
#ifndef USE_STDPERIPH_DRIVER
  #error USE_STDPERIPH_DRIVER not defined
#endif
#ifndef STM32F10X_MD
  #error STM32F10X_MD not defined
#endif
#ifndef HSE_VALUE
  #error HSE_VALUE not defined
#endif

/* Zapewnij spójność stałych definiowanych w różnych miejscach. */
#if HSE_VALUE != 8000000
  #error Gadged board uses 8 MHz external quarz.
#endif

/* Definitions for boot_10x.c */

/* Definitions for timer.c */

#define MS_TIM_N  2

/* Definitions for dac_pwm_10x.c */

#define PWM_TIM_N       3
#define PWM_TIM_CH      1
#define PWM_OUT_GPIO_N  A
#define PWM_OUT_PIN_N   6

/* Definitions for pwr_periph_10x.c */

#define POWER_LED_GPIO_N  A
#define POWER_LED_PIN_N   9
#define POWER_LED_ON      1

#define PUSH_BUTTON_GPIO_N  A
#define PUSH_BUTTON_PIN_N   0
#define PUSH_BUTTON_IRQ_N   EXTI0
#define PUSH_BUTTON_EDGE    EXTI_Trigger_Rising

#endif
