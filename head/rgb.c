#include "rgb.h"

void rgb_init(void)
{
  RCC_AHB1PeriphClockCmd(RGB_RCC, ENABLE);
  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  
  GPIO_InitStructure.GPIO_Pin = RGB_R_PIN | RGB_G_PIN | RGB_B_PIN;
  
  GPIO_Init(RGB_PORT, &GPIO_InitStructure);
  
  DR(OFF);
  DG(OFF);
  DB(OFF);  
}

void rgb_ctrl(uint8_t cfg)
{
  uint8_t set1, set2, set3;
  set1 = cfg & 0x01;
  set2 = (cfg & 0x02) >> 1;
  set3 = (cfg & 0x04) >> 2;  
  DR(!set1);
  DG(!set2);
  DB(!set3);
}
