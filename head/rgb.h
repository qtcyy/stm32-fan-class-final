#ifndef __RGB_H__
#define __RGB_H__

#include "stm32f4xx.h"
#include "led.h"

#define DR(a)                   if(a == ON) GPIO_WriteBit(RGB_PORT, RGB_R_PIN, Bit_RESET);  \
                                else  GPIO_WriteBit(RGB_PORT, RGB_R_PIN, Bit_SET)

#define DG(a)                   if(a == ON) GPIO_WriteBit(RGB_PORT, RGB_G_PIN, Bit_RESET);  \
                                else  GPIO_WriteBit(RGB_PORT, RGB_G_PIN, Bit_SET)

#define DB(a)                   if(a == ON) GPIO_WriteBit(RGB_PORT, RGB_B_PIN, Bit_RESET);  \
                                else  GPIO_WriteBit(RGB_PORT, RGB_B_PIN, Bit_SET)
                                  
#define RGB_RCC                 RCC_AHB1Periph_GPIOB
#define RGB_PORT                GPIOB
#define RGB_R_PIN               GPIO_Pin_0
#define RGB_G_PIN               GPIO_Pin_1
#define RGB_B_PIN               GPIO_Pin_2
                                  
void rgb_init();
void rgb_ctrl(uint8_t cfg);

#endif
