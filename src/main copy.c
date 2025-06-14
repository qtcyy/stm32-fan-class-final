#include "FAN.h"
#include "Infrared.h"
#include "buzzer.h"
#include "delay.h"
#include "htu21d.h"
#include "ili93xx.h"
#include "key.h"
#include "lcd.h"
#include "rgb.h"
#include "stm32f4xx.h"
#include "string.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

#define ARRAY(X) (sizeof(X) / sizeof(X[0]))
#define AUTO_MODE 1
#define MANUAL_MODE 0

static const char fan_pwm[] = {0, 33, 66, 100};
char *fan_pwm1[] = {"��", "��", "��", "��"};

#define TEMP_HIGH 30.0f
#define TEMP_MED 25.0f
#define TEMP_LOW 20.0f
#define NO_MOTION_TIMEOUT 10000

static char count = 0;
static char mode = -1;
static char human_present = 1;
unsigned char infrared_status = 0;
static char last_human_status = 1;
static uint32_t no_motion_timer = 0;
static char system_power = 1;
static char buzzer_count = 0;
static uint32_t status_output_timer = 0;

// �����������������־��е�x����
int get_centered_x(const char *text) {
  int text_len = strlen(text);
  int chinese_chars = 0;
  int english_chars = 0;

  // ͳ�������ַ���Ӣ���ַ�������
  for (int i = 0; i < text_len; i++) {
    if ((unsigned char)text[i] > 127) {
      chinese_chars++;
    } else {
      english_chars++;
    }
  }

  // �������ֿ�ȣ������ַ�16���أ�Ӣ���ַ�8����
  int estimated_width = (chinese_chars / 2) * 16 + english_chars * 8;

  // �������λ�ã�(��Ļ��� - ���ֿ��) / 2
  int centered_x = (320 - estimated_width) / 2;

  return centered_x > 0 ? centered_x : 0;
}

void delay_count(uint32_t times) {
  uint32_t temp = 0;

  while (times--) {
    temp = 1000;
    while (temp--)
      ;
  }
}

void send_human_status(void) {
  char buf[50];
  uint32_t current_time = no_motion_timer / 10;
  sprintf(buf, "Status: %s, Time: %lu.%lu s\r\n",
          human_present ? "Present" : "Absent", current_time / 10,
          current_time % 10);
  // printf(buf);
}

void send_system_status(float temp, float humi) {
  // UART״̬��� - ͨ�����ڷ���ϵͳ״̬��Ϣ
  char buf[150];
  const char *fan_status = (system_power && count > 0) ? "ON" : "OFF";
  const char *work_mode = (mode == AUTO_MODE) ? "Auto" : "Manual";

  sprintf(buf,
          "����״̬: %s, ��ǰģʽ: %s, ����ת��: %d%%, ��ǰ�¶�: %.2f��C, "
          "��ǰʪ��: %.1f%%RH\r\n",
          fan_status, work_mode, fan_pwm[count], temp, humi);
  printf(buf); // ͨ��UART�������״̬��Ϣ
}

/****************************************
 *          LCD��ʾģ�鿪ʼ             *
 ****************************************/

void update_display(float temp, float humi) {
  char buf[100];

  // ϵͳ�ر�״̬��ʾ
  if (system_power == 0) {
    sprintf(buf, "---�����ѹر�-----");
    LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x("---����K4����---"), 30 + 10 * 8, 4, 320,
                       "---����K4����---", 0x0000, 0xffff);
    // ���������ʾ��
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 10, 4, 320, "", 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 12, 4, 320, "", 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 14, 4, 320, "", 0x0000,
                       0xffff);
    return;
  }

  // ϵͳ����״̬��ʾ
  // ��ʾ����ת�ٺ͵�λ
  sprintf(buf, "����ת��: %3d%%(%s)", fan_pwm[count], fan_pwm1[count]);
  LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                     0xffff);

  // ��ʾ��ǰ����ģʽ
  sprintf(buf, "����ģʽ: %s",
          mode == AUTO_MODE ? "-�Զ�-"
                            : (mode == MANUAL_MODE ? "-�ֶ�-" : "-�Զ�-"));
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 8, 4, 320, buf, 0x0000,
                     0xffff);

  // ��ʾ��ǰ�¶�
  sprintf(buf, "��ǰ�¶�: %.2f ��C", temp);
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 10, 4, 320, buf, 0x0000,
                     0xffff);

  // ��ʾ��ǰʪ��
  sprintf(buf, "��ǰʪ��: %.1f%%RH", humi);
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 12, 4, 320, buf, 0x0000,
                     0xffff);
}

/****************************************
 *          LCD��ʾģ�����             *
 ****************************************/

/****************************************
 *          UARTͨ��ģ�鿪ʼ            *
 ****************************************/

void check_uart_command(void) {
  // ��鴮�ڽ��ջ������Ƿ����㹻������
  if (Usart_len >= 4) {
    // ����Ƿ���յ�"Auto"����
    if (strncmp((char *)USART_RX_BUF, "Auto", 4) == 0) {
      mode = AUTO_MODE;
      led_control(D2);
      printf("Mode switched to Auto\r\n");
      update_display(htu21d_t(), htu21d_h());
    }
    // ����Ƿ���յ�"Manual"����
    else if (strncmp((char *)USART_RX_BUF, "Manual", 6) == 0) {
      mode = MANUAL_MODE;
      led_control(D1);
      printf("Mode switched to Manual\r\n");
      update_display(htu21d_t(), htu21d_h());
    }
    // ��մ��ڽ��ջ�����
    clean_usart();
  }
}

/****************************************
 *          UARTͨ��ģ�����            *
 ****************************************/

void check_human_presence(void) {
  if (get_infrared_status() == 1) {
    no_motion_timer = 0;
    if (!human_present) {
      human_present = 1;
      infrared_status = 1;
    }
  } else {
    no_motion_timer++;
    if (no_motion_timer >= NO_MOTION_TIMEOUT) {
      if (human_present) {
        human_present = 0;
        infrared_status = 0;
      }
    }
  }

  if (last_human_status != human_present || (no_motion_timer % 10 == 0)) {
    send_human_status();
    last_human_status = human_present;
  }
}

/****************************************
 *         �����Զ�����ģ�鿪ʼ          *
 ****************************************/

void auto_control(float temp, float humi) {
  char new_count;

  // �����¶ȷ�Χ�Զ����ڷ��ȵ�λ
  if (temp >= TEMP_HIGH) { // �¶� >= 30��C�����ٵ�
    new_count = 3;
    // ֻ����ߵ�ʱ��3�·����������ֶ�ģʽ����һ��
    if (buzzer_count < 3) {
      buzzer_tweet();
      buzzer_count++;
    }
    rgb_ctrl(5);                 // ����RGBָʾ��Ϊ����״̬
  } else if (temp >= TEMP_MED) { // �¶� >= 25��C�����ٵ�
    new_count = 2;
    buzzer_count = 0;            // ���÷���������
    rgb_ctrl(4);                 // ����RGBָʾ��Ϊ����״̬
  } else if (temp >= TEMP_LOW) { // �¶� >= 20��C�����ٵ�
    new_count = 1;
    buzzer_count = 0; // ���÷���������
    rgb_ctrl(3);      // ����RGBָʾ��Ϊ����״̬
  } else {            // �¶� < 20��C���رշ���
    new_count = 0;
    buzzer_count = 0; // ���÷���������
    rgb_ctrl(2);      // ����RGBָʾ��Ϊ����״̬
  }

  // �����λ�����仯�����·��ȿ��ƺ���ʾ
  if (count != new_count) {
    count = new_count;
    update_display(temp, humi);
  }
}

/****************************************
 *         �����Զ�����ģ�����          *
 ****************************************/

void main(void) {
  /****************************************
   *          ϵͳ��ʼ��ģ�鿪ʼ           *
   ****************************************/

  char buf[30] = {0};
  float current_temp;
  float current_humi;

  // Ӳ��ģ���ʼ��
  delay_init(168);
  key_init();
  key_interrupt_init();
  lcd_init(FAN1);
  infrared_init();
  rgb_init();
  buzzer_init();
  led_init();
  usart_init(115200);
  htu21d_init();
  fan_pwm_init(50000 - 1, 336 - 1);

  /****************************************
   *        LCD��ʼ����ʾģ�鿪ʼ         *
   ****************************************/

  // LCD�����ʼ����ʾ - ��ʾ������ǩ�ͳ�ʼֵ
  sprintf(buf, "Fan PWM:%3d%%", fan_pwm[count]);
  LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                     0xffff); // ��ʾ��ʼPWMֵ
  LCDDrawFont16_Next(get_centered_x("Mode:"), 30 + 10 * 8, 4, 320,
                     "Mode:", 0x0000, 0xffff); // ��ʾģʽ��ǩ
  LCDDrawFont16_Next(get_centered_x("Temp:"), 30 + 10 * 10, 4, 320,
                     "Temp:", 0x0000, 0xffff); // ��ʾ�¶ȱ�ǩ
  LCDDrawFont16_Next(get_centered_x("Humi:"), 30 + 10 * 12, 4, 320,
                     "Humi:", 0x0000, 0xffff); // ��ʾʪ�ȱ�ǩ

  /****************************************
   *        LCD��ʼ����ʾģ�����         *
   ****************************************/

  /****************************************
   *          ϵͳ��ʼ��ģ�����           *
   ****************************************/

  while (1) {
    // ����״̬�����ʱ��
    status_output_timer++;

    /****************************************
     *          ��������ģ�鿪ʼ            *
     ****************************************/

    // K4������ϵͳ��Դ���ؿ���
    if (key_interrupt_flag) {
      delay_count(50); // ����������ʱ
      char current_key = key_value;
      key_interrupt_flag = 0;

      if (current_key == K4_PREESED) {
        system_power = !system_power; // �л�ϵͳ��Դ״̬
        buzzer_tweet();               // ����ȷ����
        if (system_power == 0) {
          count = 0;                       // �ر�ʱ���÷��ȵ�λ
          fan_pwm_control(fan_pwm[count]); // ֹͣ����
          rgb_ctrl(2);                     // ����RGBΪ����״̬
        }
        update_display(htu21d_t(), htu21d_h());
        continue;
      }
    }

    if (system_power == 0) {
      // ��ʹϵͳ�ر�ҲҪ���״̬��Ϣ
      if (status_output_timer >=
          1000) { // ÿ�����һ�� (������ѭ����Լ1msִ��һ��)
        send_system_status(htu21d_t(), htu21d_h());
        status_output_timer = 0;
      }
      delay_ms(100);
      continue;
    }

    // UARTͨ�ż�� - �����ڽ��յ�ģʽ�л�����
    check_uart_command();
    check_human_presence();

    if (human_present) {
      /****************************************
       *          ��ʪ�ȼ��ģ�鿪ʼ           *
       ****************************************/

      // ��ȡ��ǰ��ʪ�ȴ���������
      current_temp = htu21d_t();
      current_humi = htu21d_h();

      /****************************************
       *          ��ʪ�ȼ��ģ�����           *
       ****************************************/

      // K3������ģʽ�л���K1/K2�������ֶ���λ����
      if (key_interrupt_flag) {
        delay_count(50); // ����������ʱ
        char current_key = key_value;
        key_interrupt_flag = 0;

        if (current_key == K3_PREESED) {
          // K3�������Զ�/�ֶ�ģʽ�л�
          if (mode == AUTO_MODE) {
            mode = MANUAL_MODE; // �л����ֶ�ģʽ
            led_control(D1);    // �ֶ�ģʽָʾ��
          } else {
            mode = AUTO_MODE; // �л����Զ�ģʽ
            led_control(D2);  // �Զ�ģʽָʾ��
          }
          buzzer_tweet(); // ����ȷ����
          update_display(current_temp, current_humi);
        } else if (mode == MANUAL_MODE && current_key == K1_PREESED) {
          // K1�������ֶ�ģʽ�����ӷ��ȵ�λ
          count++;
          buzzer_tweet(); // ����ȷ����
          if (count >= ARRAY(fan_pwm))
            count = ARRAY(fan_pwm) - 1; // �������λ
          update_display(current_temp, current_humi);
        } else if (mode == MANUAL_MODE && current_key == K2_PREESED) {
          // K2�������ֶ�ģʽ�¼��ٷ��ȵ�λ
          if (count > 0) {
            count--; // ���ٵ�λ
          }
          buzzer_tweet(); // ����ȷ����
          update_display(current_temp, current_humi);
        }
      }

      /****************************************
       *          ��������ģ�����            *
       ****************************************/

      // ���ݵ�ǰģʽִ�з��ȿ��Ʋ���
      if (mode == AUTO_MODE) {
        // �Զ�ģʽ�������¶��Զ����ڷ���ת��
        auto_control(current_temp, current_humi);
      } else {
        // �ֶ�ģʽ�������û��趨�ĵ�λ���Ʒ��Ⱥ�ָʾ��
        if (count == 3) { // ��ߵ�λ��100%��
          if (buzzer_count < 3) {
            buzzer_tweet(); // ��ߵ�ʱ��������3������
            buzzer_count++;
          }
          rgb_ctrl(5);           // ���ٵ�RGBָʾ��
        } else if (count == 2) { // �е�λ��66%��
          buzzer_count = 0;      // ���÷���������
          rgb_ctrl(4);           // ���ٵ�RGBָʾ��
        } else if (count == 1) { // �͵�λ��33%��
          buzzer_count = 0;      // ���÷���������
          rgb_ctrl(3);           // ���ٵ�RGBָʾ��
        } else if (count == 0) { // �رյ�λ��0%��
          buzzer_count = 0;      // ���÷���������
          rgb_ctrl(2);           // ֹͣ��RGBָʾ��
        }
      }

      if (no_motion_timer % 100 == 0) {
        update_display(current_temp, current_humi);
        // send_human_status();
      }

    } else if (mode == AUTO_MODE) {
      // ����״̬�µ��Զ�ģʽ�����������¶ȿ��Ʒ���
      auto_control(current_temp, current_humi);
      if (no_motion_timer % 100 == 0) {
        update_display(current_temp, current_humi);
        // send_human_status();
      }
    }

    // ÿ�����һ��ϵͳ״̬��Ϣ
    if (status_output_timer >= 3) { // ������ѭ����Լ1msִ��һ��
      send_system_status(htu21d_t(), htu21d_h());
      status_output_timer = 0;
    }

    fan_pwm_control(fan_pwm[count]);
    delay_ms(1); // ȷ����ѭ��Լ1msִ��һ��
  }
}

/*
else {
    count = 0;
    fan_pwm_control(fan_pwm[count]);
    update_display(current_temp, current_humi);
  }
*/