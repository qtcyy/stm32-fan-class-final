/*********************************************************************************************
 * 文件：FAN.c
 * 作者：Lixm 2017.10.17
 * 说明：风扇驱动代码
 * 修改：fuyou 2018.8.25 修正风扇控制驱动
 * 注释：
 *********************************************************************************************/

/*********************************************************************************************
 * 头文件
 *********************************************************************************************/
#include "FAN.h"

/*********************************************************************************************
 * 名称：fan_init()
 * 功能：风扇传感器初始化
 * 参数：无
 * 返回：无
 *********************************************************************************************/
void fan_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure; // 定义一个GPIO_InitTypeDef类型的结构体
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,
                         ENABLE); // 开启风扇传感器相关的GPIO外设时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;        // 选择要控制的GPIO引脚
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   // 设置引脚的输出类型为推挽
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;    // 设置引脚模式为输出模式
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;   // 设置引脚为下拉模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 设置引脚速率为2MHz

  GPIO_Init(GPIOE, &GPIO_InitStructure); // 初始化GPIO配置
  GPIO_ResetBits(GPIOE, GPIO_Pin_5);
}

/*********************************************************************************************
 * 名称：void fan_control(unsigned char cmd)
 * 功能：风扇控制驱动
 * 参数：控制命令
 * 返回：无
 *********************************************************************************************/
void fan_control(unsigned char cmd) {
  if (cmd & 0x01)
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
  else
    GPIO_ResetBits(GPIOE, GPIO_Pin_5);
}

static u32 cycle; // 这个值不要小于100，否则占空比不准确
/*********************************************************************************************
 * 名称：fan_pwm_init()
 * 功能：风扇传感器PWM初始化  PE5 连接 TIM9——CH1  16位定时器
 * 参数：arr：自动重装值  psc：时钟预分频数
 * 返回：无
 *********************************************************************************************/
void fan_pwm_init(u32 arr, u32 psc) {
  cycle = arr + 1; // 用来计算占空比。需要加1
  // 此部分需手动修改IO口设置
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
  TIM_OCInitTypeDef TIM_OCInitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);  // TIM9时钟使能
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); // 使能PORTF时钟

  GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_TIM9); // GPIOE5
                                                          // 复用为定时器9

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;          // GPIOE5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       // 复用功能
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 速度100MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽复用输出
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);             // 初始化PE5

  TIM_TimeBaseStructure.TIM_Prescaler =
      psc; // 定时器分频,定时器9挂载到APB2，为168MHZ，如果这里分频到1MHZ，设置为167
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
  TIM_TimeBaseStructure.TIM_Period = arr;                     // 自动重装载值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure); // 初始化定时器9

  // 初始化TIM9 Channel1 PWM模式
  TIM_OCInitStructure.TIM_OCMode =
      TIM_OCMode_PWM1; // 选择定时器模式:TIM脉冲宽度调制模式
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 比较输出使能
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     // 输出极性
  TIM_OC1Init(TIM9, &TIM_OCInitStructure);                      // 初始化通道1

  TIM_OC1PreloadConfig(TIM9,
                       TIM_OCPreload_Enable); // 使能TIM在CCR1上的预装载寄存器

  TIM_ARRPreloadConfig(TIM9, ENABLE); // ARPE使能
  TIM_Cmd(TIM9, ENABLE);              // 使能TIM
}

/*********************************************************************************************
 * 名称：fan_pwm_control
 * 功能：风扇PWM驱动控制
 * 参数：pwm 占空比 0-100
 * 返回：无
 *********************************************************************************************/
void fan_pwm_control(uint32_t pwm) {
  uint32_t _pwm = cycle / 100 * pwm;
  TIM_SetCompare1(TIM9, _pwm); // 修改比较值，修改占空比
}