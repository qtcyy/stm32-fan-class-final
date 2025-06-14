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
  char buf[150];
  const char *fan_status = (system_power && count > 0) ? "ON" : "OFF";
  const char *work_mode = (mode == AUTO_MODE) ? "Auto" : "Manual";

  sprintf(buf,
          "����״̬: %s, ��ǰģʽ: %s, ����ת��: %d%%, ��ǰ�¶�: %.2f��C, "
          "��ǰʪ��: %.1f%%RH\r\n",
          fan_status, work_mode, fan_pwm[count], temp, humi);
  printf(buf);
}

void update_display(float temp, float humi) {
  char buf[100];

  if (system_power == 0) {
    sprintf(buf, "---�����ѹر�-----");
    LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x("---����K4����---"), 30 + 10 * 8, 4, 320,
                       "---����K4����---", 0x0000, 0xffff);
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 10, 4, 320, "", 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 12, 4, 320, "", 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 14, 4, 320, "", 0x0000,
                       0xffff);
    return;
  }

  sprintf(buf, "����ת��: %3d%%(%s)", fan_pwm[count], fan_pwm1[count]);

  LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                     0xffff);

  sprintf(buf, "����ģʽ: %s",
          mode == AUTO_MODE ? "-�Զ�-"
                            : (mode == MANUAL_MODE ? "-�ֶ�-" : "-�Զ�-"));

  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 8, 4, 320, buf, 0x0000,
                     0xffff);
  // printf(buf);
  // printf("\r\n");

  sprintf(buf, "��ǰ�¶�: %.2f ��C", temp);
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 10, 4, 320, buf, 0x0000,
                     0xffff);

  sprintf(buf, "��ǰʪ��: %.1f%%RH", humi);
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 12, 4, 320, buf, 0x0000,
                     0xffff);
}

void check_uart_command(void) {
  if (Usart_len >= 4) {
    if (strncmp((char *)USART_RX_BUF, "Auto", 4) == 0) {
      mode = AUTO_MODE;
      led_control(D2);
      printf("Mode switched to Auto\r\n");
      update_display(htu21d_t(), htu21d_h());
    } else if (strncmp((char *)USART_RX_BUF, "Manual", 6) == 0) {
      mode = MANUAL_MODE;
      led_control(D1);
      printf("Mode switched to Manual\r\n");
      update_display(htu21d_t(), htu21d_h());
    }
    clean_usart();
  }
}

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

void auto_control(float temp, float humi) {
  char new_count;
  if (temp >= TEMP_HIGH) {
    new_count = 3;
    // ֻ����ߵ�ʱ��3�·����������ֶ�ģʽ����һ��
    if (buzzer_count < 3) {
      buzzer_tweet();
      buzzer_count++;
    }
    rgb_ctrl(5);
  } else if (temp >= TEMP_MED) {
    new_count = 2;
    buzzer_count = 0; // ���÷���������
    rgb_ctrl(4);
  } else if (temp >= TEMP_LOW) {
    new_count = 1;
    buzzer_count = 0; // ���÷���������
    rgb_ctrl(3);
  } else {
    new_count = 0;
    buzzer_count = 0; // ���÷���������
    rgb_ctrl(2);
  }
  if (count != new_count) {
    count = new_count;
    update_display(temp, humi);
  }
}

void main(void) {
  char buf[30] = {0};
  float current_temp;
  float current_humi;

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

  sprintf(buf, "Fan PWM:%3d%%", fan_pwm[count]);
  LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                     0xffff);
  LCDDrawFont16_Next(get_centered_x("Mode:"), 30 + 10 * 8, 4, 320,
                     "Mode:", 0x0000, 0xffff);
  LCDDrawFont16_Next(get_centered_x("Temp:"), 30 + 10 * 10, 4, 320,
                     "Temp:", 0x0000, 0xffff);
  LCDDrawFont16_Next(get_centered_x("Humi:"), 30 + 10 * 12, 4, 320,
                     "Humi:", 0x0000, 0xffff);

  while (1) {
    // ����״̬�����ʱ��
    status_output_timer++;

    if (key_interrupt_flag) {
      delay_count(10); // ���ٰ���������ʱ��50��10
      char current_key = key_value;
      key_interrupt_flag = 0;

      if (current_key == K4_PREESED) {
        system_power = !system_power;
        buzzer_tweet();
        if (system_power == 0) {
          count = 0;
          fan_pwm_control(fan_pwm[count]);
          rgb_ctrl(2);
        }
        // �ӳ�LCD���£���������
        continue;
      }
    }

    if (system_power == 0) {
      // ��ʹϵͳ�ر�ҲҪ���״̬��Ϣ
      if (status_output_timer >= 1000) {
        send_system_status(htu21d_t(), htu21d_h());
        status_output_timer = 0;
      }
      delay_ms(10); // ������ʱ��100ms��10ms�������Ӧ�ٶ�
      continue;
    }

    check_uart_command();
    check_human_presence();

    if (human_present) {
      current_temp = htu21d_t();
      current_humi = htu21d_h();

      if (key_interrupt_flag) {
        delay_count(10); // ���ٰ���������ʱ
        char current_key = key_value;
        key_interrupt_flag = 0;

        if (current_key == K3_PREESED) {
          if (mode == AUTO_MODE) {
            mode = MANUAL_MODE;
            led_control(D1);
          } else {
            mode = AUTO_MODE;
            led_control(D2);
          }
          buzzer_tweet();
          // �����Ҫ������ʾ��������������
        } else if (mode == MANUAL_MODE && current_key == K1_PREESED) {
          count++;
          buzzer_tweet();
          if (count >= ARRAY(fan_pwm))
            count = ARRAY(fan_pwm) - 1;
          // �����Ҫ������ʾ��������������
        } else if (mode == MANUAL_MODE && current_key == K2_PREESED) {
          if (count > 0) {
            count--;
          }
          buzzer_tweet();
          // �����Ҫ������ʾ��������������
        }
      }

      if (mode == AUTO_MODE) {
        auto_control(current_temp, current_humi);
      } else {
        if (count == 3) {
          if (buzzer_count < 3) {
            buzzer_tweet();
            buzzer_count++;
          }
          rgb_ctrl(5);
        } else if (count == 2) {
          buzzer_count = 0;
          rgb_ctrl(4);
        } else if (count == 1) {
          buzzer_count = 0;
          rgb_ctrl(3);
        } else if (count == 0) {
          buzzer_count = 0;
          rgb_ctrl(2);
        }
      }

      // ����LCD����Ƶ�ʣ������Ӧ�ٶ�
      if (no_motion_timer % 500 == 0) { // ��100��Ϊ500�����ٸ���Ƶ��
        update_display(current_temp, current_humi);
      }

    } else if (mode == AUTO_MODE) {
      auto_control(current_temp, current_humi);
      if (no_motion_timer % 500 == 0) { // ��100��Ϊ500�����ٸ���Ƶ��
        update_display(current_temp, current_humi);
      }
    }

    // ����״̬���Ƶ��
    if (status_output_timer >= 100) { // ��1��Ϊ100������״̬���Ƶ��
      send_system_status(htu21d_t(), htu21d_h());
      status_output_timer = 0;
    }

    fan_pwm_control(fan_pwm[count]);
    // �Ƴ���ѭ����ʱ�������Ӧ�ٶ�
  }
}