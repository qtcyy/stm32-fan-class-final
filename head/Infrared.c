/*********************************************************************************************
* 文件：Infrared.c
* 作者：Lixm 2017.10.17
* 说明：人体红外驱动代码
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "Infrared.h"

/*********************************************************************************************
* 名称：infrared_init()
* 功能：人体红外传感器初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void infrared_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;                          //定义一个GPIO_InitTypeDef类型的结构体
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);         //开启人体红外相关的GPIO外设时钟
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                     //选择要控制的GPIO引脚
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //设置引脚的输出类型为推挽
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;                  //设置引脚模式为输入模式
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;                //设置引脚为下拉模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;              //设置引脚速率为2MHz
  
  GPIO_Init(GPIOC, &GPIO_InitStructure);                        //初始化GPIO配置
}

/*********************************************************************************************
* 名称：unsigned char get_infrared_status(void)
* 功能：获取人体红外传感器状态
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
unsigned char get_infrared_status(void)
{ 
  if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8))                   //判断人体红外引脚状态
    return 1;
  else
    return 0;
}