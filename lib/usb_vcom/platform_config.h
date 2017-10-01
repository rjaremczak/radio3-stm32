#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include <stm32f10x.h>

#define RCC_APB2Periph_ALLGPIO              (RCC_APB2Periph_GPIOA \
                                               | RCC_APB2Periph_GPIOB \
                                               | RCC_APB2Periph_GPIOC \
                                               | RCC_APB2Periph_GPIOD \
                                               | RCC_APB2Periph_GPIOE )

/*Unique Devices IDs register set*/

/*STM32F1x*/

#define         ID1          (0x1FFFF7E8)
#define         ID2          (0x1FFFF7EC)
#define         ID3          (0x1FFFF7F0)


#endif /* __PLATFORM_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
