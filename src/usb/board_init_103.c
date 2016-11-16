#include <active_check.h>
#include <board_def.h>
#include <board_init.h>

/** STM32F103 initialization **/

static GPIO_TypeDef * const gpio[] = {
  GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG
};
static const int gpioCount = sizeof(gpio)/sizeof(gpio[0]);
static const uint32_t allRccGpio = RCC_APB2Periph_GPIOA |
                                   RCC_APB2Periph_GPIOB |
                                   RCC_APB2Periph_GPIOC |
                                   RCC_APB2Periph_GPIOD |
                                   RCC_APB2Periph_GPIOE |
                                   RCC_APB2Periph_GPIOF |
                                   RCC_APB2Periph_GPIOG;

/* Configure all leads as analog inputs with trigger off, which
   reduces power consumption and electromagnetic interferences. */
void AllPinsDisable() {
  GPIO_InitTypeDef GPIO_InitStruct;
  int i;

  RCC_APB2PeriphClockCmd(allRccGpio, ENABLE);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_InitStruct.GPIO_Speed = 0;
  for (i = 0; i < gpioCount; ++i)
    GPIO_Init(gpio[i], &GPIO_InitStruct);

  RCC_APB2PeriphClockCmd(allRccGpio, DISABLE);
}

#ifndef HSE_BYPASS
  #define HSE_BYPASS  0
#endif

static const int maxAttempts = 1000000;

int ClockConfigure(int sysclk_MHz) {
  RCC_DeInit();
  if (HSE_BYPASS)
    RCC_HSEConfig(RCC_HSE_Bypass);
  else
    RCC_HSEConfig(RCC_HSE_ON);
  /* Try maximum HSEStartUp_TimeOut == 0x0500 times. */
  if (RCC_WaitForHSEStartUp() == ERROR)
    return -1;

  /* PLL, PLLCLK = HSE * PLLMUL or PLLCLK = (HSE / 2) * PLLMUL */
  if (HSE_VALUE == 8000000 && sysclk_MHz == 48)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
  else if (HSE_VALUE == 8000000 && sysclk_MHz == 72)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

  else if (HSE_VALUE == 12000000 && sysclk_MHz == 48)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_4);
  else if (HSE_VALUE == 12000000 && sysclk_MHz == 72)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);

  else if (HSE_VALUE == 16000000 && sysclk_MHz == 48)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_3);
  else if (HSE_VALUE == 16000000 && sysclk_MHz == 72)
    RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_9);

  else
    return -1;

  RCC_PLLCmd(ENABLE);
  active_check(RCC_GetFlagStatus(RCC_FLAG_PLLRDY), maxAttempts);

  FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
  if (sysclk_MHz > 48)
    FLASH_SetLatency(FLASH_Latency_2);
  else if (sysclk_MHz > 24)
    FLASH_SetLatency(FLASH_Latency_1);
  else
    FLASH_SetLatency(FLASH_Latency_0);

  /* Set AHB prescaler, HCLK = SYSCLK. */
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  /* Set APB1 prescaler, PCLK1 = HCLK / 2. */
  RCC_PCLK1Config(RCC_HCLK_Div2);
  /* Set APB2 prescaler, PCLK2 = HCLK. */
  RCC_PCLK2Config(RCC_HCLK_Div1);

  /* Set SYSCLK = PLLCLK. */
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  active_check(RCC_GetSYSCLKSource() == 0x08, maxAttempts);

  RCC_HSICmd(DISABLE);

  return 0;
}

int ClockReenable(void) {
  if (HSE_BYPASS)
    RCC_HSEConfig(RCC_HSE_Bypass);
  else
    RCC_HSEConfig(RCC_HSE_ON);

  if (RCC_WaitForHSEStartUp() == ERROR)
    return -1;

  RCC_PLLCmd(ENABLE);
  active_check(RCC_GetFlagStatus(RCC_FLAG_PLLRDY), maxAttempts);

  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  active_check(RCC_GetSYSCLKSource() == 0x08, maxAttempts);

  RCC_HSICmd(DISABLE);

  return 0;
}
