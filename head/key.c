/*********************************************************************************************
 * �ļ���key.c
 * ���ߣ�chennian 2017.11.02
 * ˵����K1 ��Ӧ���� PB12
 *       K2 ��Ӧ�ܽ� PB13
 *       K3 ��Ӧ���� PB14
 *       K4 ��Ӧ�ܽ� PB15
 * ���ܣ�������������
 * �޸ģ�zhoucj   2017.12.18 ����ע��
 *********************************************************************************************/
/*********************************************************************************************
 * ͷ�ļ�
 *********************************************************************************************/
#include "key.h"

volatile char key_interrupt_flag = 0;
volatile char key_value = 0;

/*********************************************************************************************
 * ���ƣ�key_init
 * ���ܣ������ܽų�ʼ��
 * ��������
 * ���أ���
 * �޸ģ���
 *********************************************************************************************/
void key_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure; // ����һ��GPIO_InitTypeDef���͵Ľṹ��
  RCC_AHB1PeriphClockCmd(K1_CLK | K2_CLK | K3_CLK | K4_CLK,
                         ENABLE); // ����KEY��ص�GPIO����ʱ��

  GPIO_InitStructure.GPIO_Pin =
      K1_PIN | K2_PIN | K3_PIN | K4_PIN;           // ѡ��Ҫ���Ƶ�GPIO����
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;   // �������ŵ��������Ϊ�������
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;     // ��������ģʽΪ����ģʽ
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;     // ��������Ϊ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // ������������Ϊ2MHz

  GPIO_Init(K1_PORT, &GPIO_InitStructure); // ��ʼ��GPIOB����
  GPIO_Init(K2_PORT, &GPIO_InitStructure); // ��ʼ��GPIOB����
  GPIO_Init(K3_PORT, &GPIO_InitStructure); // ��ʼ��GPIOB����
  GPIO_Init(K4_PORT, &GPIO_InitStructure); // ��ʼ��GPIOB����
}

/*********************************************************************************************
 * ���ƣ�get_key_status
 * ���ܣ������ܽų�ʼ��
 * ��������
 * ���أ�key_status
 * �޸ģ�
 *********************************************************************************************/
char get_key_status(void) {
  char key_status = 0;
  if (GPIO_ReadInputDataBit(K1_PORT, K1_PIN) == 0) // �ж�PB12���ŵ�ƽ״̬
    key_status |= K1_PREESED;                      // �͵�ƽkey_status bit0λ��1
  if (GPIO_ReadInputDataBit(K2_PORT, K2_PIN) == 0) // �ж�PB13���ŵ�ƽ״̬
    key_status |= K2_PREESED;                      // �͵�ƽkey_status bit1λ��1
  if (GPIO_ReadInputDataBit(K3_PORT, K3_PIN) == 0) // �ж�PB14���ŵ�ƽ״̬
    key_status |= K3_PREESED;                      // �͵�ƽkey_status bit2λ��1
  if (GPIO_ReadInputDataBit(K4_PORT, K4_PIN) == 0) // �ж�PB15���ŵ�ƽ״̬
    key_status |= K4_PREESED;                      // �͵�ƽkey_status bit3λ��1
  return key_status;
}

void key_interrupt_init(void) {
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource12);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource13);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource14);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource15);

  EXTI_InitStructure.EXTI_Line =
      EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void EXTI15_10_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
    EXTI_ClearITPendingBit(EXTI_Line12);
    key_value = K1_PREESED;
    key_interrupt_flag = 1;
  }
  if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
    EXTI_ClearITPendingBit(EXTI_Line13);
    key_value = K2_PREESED;
    key_interrupt_flag = 1;
  }
  if (EXTI_GetITStatus(EXTI_Line14) != RESET) {
    EXTI_ClearITPendingBit(EXTI_Line14);
    key_value = K3_PREESED;
    key_interrupt_flag = 1;
  }
  if (EXTI_GetITStatus(EXTI_Line15) != RESET) {
    EXTI_ClearITPendingBit(EXTI_Line15);
    key_value = K4_PREESED;
    key_interrupt_flag = 1;
  }
}
