/*********************************************************************************************
 * �ļ���FAN.c
 * ���ߣ�Lixm 2017.10.17
 * ˵����������������
 * �޸ģ�fuyou 2018.8.25 �������ȿ�������
 * ע�ͣ�
 *********************************************************************************************/

/*********************************************************************************************
 * ͷ�ļ�
 *********************************************************************************************/
#include "FAN.h"

/*********************************************************************************************
 * ���ƣ�fan_init()
 * ���ܣ����ȴ�������ʼ��
 * ��������
 * ���أ���
 *********************************************************************************************/
void fan_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure; // ����һ��GPIO_InitTypeDef���͵Ľṹ��
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,
                         ENABLE); // �������ȴ�������ص�GPIO����ʱ��

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;        // ѡ��Ҫ���Ƶ�GPIO����
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   // �������ŵ��������Ϊ����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;    // ��������ģʽΪ���ģʽ
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;   // ��������Ϊ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // ������������Ϊ2MHz

  GPIO_Init(GPIOE, &GPIO_InitStructure); // ��ʼ��GPIO����
  GPIO_ResetBits(GPIOE, GPIO_Pin_5);
}

/*********************************************************************************************
 * ���ƣ�void fan_control(unsigned char cmd)
 * ���ܣ����ȿ�������
 * ��������������
 * ���أ���
 *********************************************************************************************/
void fan_control(unsigned char cmd) {
  if (cmd & 0x01)
    GPIO_SetBits(GPIOE, GPIO_Pin_5);
  else
    GPIO_ResetBits(GPIOE, GPIO_Pin_5);
}

static u32 cycle; // ���ֵ��ҪС��100������ռ�ձȲ�׼ȷ
/*********************************************************************************************
 * ���ƣ�fan_pwm_init()
 * ���ܣ����ȴ�����PWM��ʼ��  PE5 ���� TIM9����CH1  16λ��ʱ��
 * ������arr���Զ���װֵ  psc��ʱ��Ԥ��Ƶ��
 * ���أ���
 *********************************************************************************************/
void fan_pwm_init(u32 arr, u32 psc) {
  cycle = arr + 1; // ��������ռ�ձȡ���Ҫ��1
  // �˲������ֶ��޸�IO������
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
  TIM_OCInitTypeDef TIM_OCInitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);  // TIM9ʱ��ʹ��
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); // ʹ��PORTFʱ��

  GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_TIM9); // GPIOE5
                                                          // ����Ϊ��ʱ��9

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;          // GPIOE5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       // ���ù���
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // �ٶ�100MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // ���츴�����
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       // ����
  GPIO_Init(GPIOE, &GPIO_InitStructure);             // ��ʼ��PE5

  TIM_TimeBaseStructure.TIM_Prescaler =
      psc; // ��ʱ����Ƶ,��ʱ��9���ص�APB2��Ϊ168MHZ����������Ƶ��1MHZ������Ϊ167
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
  TIM_TimeBaseStructure.TIM_Period = arr;                     // �Զ���װ��ֵ
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure); // ��ʼ����ʱ��9

  // ��ʼ��TIM9 Channel1 PWMģʽ
  TIM_OCInitStructure.TIM_OCMode =
      TIM_OCMode_PWM1; // ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // �Ƚ����ʹ��
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     // �������
  TIM_OC1Init(TIM9, &TIM_OCInitStructure);                      // ��ʼ��ͨ��1

  TIM_OC1PreloadConfig(TIM9,
                       TIM_OCPreload_Enable); // ʹ��TIM��CCR1�ϵ�Ԥװ�ؼĴ���

  TIM_ARRPreloadConfig(TIM9, ENABLE); // ARPEʹ��
  TIM_Cmd(TIM9, ENABLE);              // ʹ��TIM
}

/*********************************************************************************************
 * ���ƣ�fan_pwm_control
 * ���ܣ�����PWM��������
 * ������pwm ռ�ձ� 0-100
 * ���أ���
 *********************************************************************************************/
void fan_pwm_control(uint32_t pwm) {
  uint32_t _pwm = cycle / 100 * pwm;
  TIM_SetCompare1(TIM9, _pwm); // �޸ıȽ�ֵ���޸�ռ�ձ�
}