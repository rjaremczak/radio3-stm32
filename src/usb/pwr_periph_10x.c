#include <board_def.h>
#include <irq.h>
#include <pwr_periph.h>
#include <usbd_power.h>

#define GPIO_POWER_LED  xcat(GPIO, POWER_LED_GPIO_N)
#define PIN_POWER_LED   xcat(GPIO_Pin_, POWER_LED_PIN_N)
#define RCC_POWER_LED   xcat(RCC_APB2Periph_GPIO, POWER_LED_GPIO_N)
#define POWER_LED_OFF   (!(POWER_LED_ON))

#define GPIO_PUSH_BUTTON        xcat(GPIO, PUSH_BUTTON_GPIO_N)
#define PIN_PUSH_BUTTON         xcat(GPIO_Pin_, PUSH_BUTTON_PIN_N)
#define RCC_PUSH_BUTTON         xcat(RCC_APB2Periph_GPIO, PUSH_BUTTON_GPIO_N)
#define PORT_SRC_PUSH_BUTTON    xcat(GPIO_PortSourceGPIO, PUSH_BUTTON_GPIO_N)
#define PIN_SRC_PUSH_BUTTON     xcat(GPIO_PinSource, PUSH_BUTTON_PIN_N)
#define PUSH_BUTTON_EXTI_LINE   xcat(EXTI_Line, PUSH_BUTTON_PIN_N)
#define PUSH_BUTTON_EXTI_IRQn   xcat(PUSH_BUTTON_IRQ_N, _IRQn)
#define PUSH_BUTTON_IRQHandler  xcat(PUSH_BUTTON_IRQ_N, _IRQHandler)

void PowerLEDconfigure() {
  GPIO_InitTypeDef GPIO_InitStruct;

  RCC_APB2PeriphClockCmd(RCC_POWER_LED, ENABLE);

  GPIO_WriteBit(GPIO_POWER_LED, PIN_POWER_LED, POWER_LED_OFF);

  GPIO_InitStruct.GPIO_Pin = PIN_POWER_LED;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIO_POWER_LED, &GPIO_InitStruct);
}

void PowerLEDon() {
  GPIO_WriteBit(GPIO_POWER_LED, PIN_POWER_LED, POWER_LED_ON);
}

void PowerLEDoff() {
  GPIO_WriteBit(GPIO_POWER_LED, PIN_POWER_LED, POWER_LED_OFF);
}

int PowerLEDstate() {
  return GPIO_ReadOutputDataBit(GPIO_POWER_LED,
                                PIN_POWER_LED) == POWER_LED_ON;
}

void WakeupButtonConfigure() {
  GPIO_InitTypeDef GPIO_InitStruct;
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  RCC_APB2PeriphClockCmd(RCC_PUSH_BUTTON | RCC_APB2Periph_AFIO,
                         ENABLE);

  GPIO_InitStruct.GPIO_Pin = PIN_PUSH_BUTTON;
  if (PUSH_BUTTON_EDGE == EXTI_Trigger_Falling)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
  else if (PUSH_BUTTON_EDGE == EXTI_Trigger_Rising)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
  else
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStruct.GPIO_Speed = 0;
  GPIO_Init(GPIO_PUSH_BUTTON, &GPIO_InitStruct);

  GPIO_EXTILineConfig(PORT_SRC_PUSH_BUTTON, PIN_SRC_PUSH_BUTTON);

  EXTI_ClearITPendingBit(PUSH_BUTTON_EXTI_LINE);

  EXTI_InitStruct.EXTI_Line = PUSH_BUTTON_EXTI_LINE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = PUSH_BUTTON_EDGE;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStruct);

  NVIC_InitStruct.NVIC_IRQChannel = PUSH_BUTTON_EXTI_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = HIGH_IRQ_PRIO;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

void PUSH_BUTTON_IRQHandler(void) {
  if (EXTI_GetITStatus(PUSH_BUTTON_EXTI_LINE)) {
    PWRremoteWakeUp();
    /* Clear the pending bit at the end to debounce the button. */
    EXTI_ClearITPendingBit(PUSH_BUTTON_EXTI_LINE);
  }
}
