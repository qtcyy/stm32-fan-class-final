#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "stm32f4xx.h"
#include "led.h"
#include "delay.h"

#define BUZZER_RCC              RCC_AHB1Periph_GPIOD
#define BUZZER_PORT             GPIOD
#define BUZZER_PIN              GPIO_Pin_6

#define BUZZER_CTRL(a)          if(!a) GPIO_SetBits(BUZZER_PORT, BUZZER_PIN); \
                                else GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN)

void buzzer_init(void);
void buzzer_tweet(void);

#endif

