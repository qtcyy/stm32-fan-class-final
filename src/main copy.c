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
char *fan_pwm1[] = {"关", "低", "中", "高"};

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

// 辅助函数：计算文字居中的x坐标
int get_centered_x(const char *text) {
  int text_len = strlen(text);
  int chinese_chars = 0;
  int english_chars = 0;

  // 统计中文字符和英文字符的数量
  for (int i = 0; i < text_len; i++) {
    if ((unsigned char)text[i] > 127) {
      chinese_chars++;
    } else {
      english_chars++;
    }
  }

  // 估算文字宽度：中文字符16像素，英文字符8像素
  int estimated_width = (chinese_chars / 2) * 16 + english_chars * 8;

  // 计算居中位置：(屏幕宽度 - 文字宽度) / 2
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
  // UART状态输出 - 通过串口发送系统状态信息
  char buf[150];
  const char *fan_status = (system_power && count > 0) ? "ON" : "OFF";
  const char *work_mode = (mode == AUTO_MODE) ? "Auto" : "Manual";

  sprintf(buf,
          "启动状态: %s, 当前模式: %s, 风扇转速: %d%%, 当前温度: %.2f°C, "
          "当前湿度: %.1f%%RH\r\n",
          fan_status, work_mode, fan_pwm[count], temp, humi);
  printf(buf); // 通过UART串口输出状态信息
}

/****************************************
 *          LCD显示模块开始             *
 ****************************************/

void update_display(float temp, float humi) {
  char buf[100];

  // 系统关闭状态显示
  if (system_power == 0) {
    sprintf(buf, "---风扇已关闭-----");
    LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x("---按下K4启动---"), 30 + 10 * 8, 4, 320,
                       "---按下K4启动---", 0x0000, 0xffff);
    // 清空其他显示行
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 10, 4, 320, "", 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 12, 4, 320, "", 0x0000,
                       0xffff);
    LCDDrawFont16_Next(get_centered_x(""), 30 + 10 * 14, 4, 320, "", 0x0000,
                       0xffff);
    return;
  }

  // 系统运行状态显示
  // 显示风扇转速和档位
  sprintf(buf, "风扇转速: %3d%%(%s)", fan_pwm[count], fan_pwm1[count]);
  LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                     0xffff);

  // 显示当前工作模式
  sprintf(buf, "风扇模式: %s",
          mode == AUTO_MODE ? "-自动-"
                            : (mode == MANUAL_MODE ? "-手动-" : "-自动-"));
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 8, 4, 320, buf, 0x0000,
                     0xffff);

  // 显示当前温度
  sprintf(buf, "当前温度: %.2f °C", temp);
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 10, 4, 320, buf, 0x0000,
                     0xffff);

  // 显示当前湿度
  sprintf(buf, "当前湿度: %.1f%%RH", humi);
  LCDDrawFont16_Next(get_centered_x(buf), 30 + 10 * 12, 4, 320, buf, 0x0000,
                     0xffff);
}

/****************************************
 *          LCD显示模块结束             *
 ****************************************/

/****************************************
 *          UART通信模块开始            *
 ****************************************/

void check_uart_command(void) {
  // 检查串口接收缓冲区是否有足够的数据
  if (Usart_len >= 4) {
    // 检查是否接收到"Auto"命令
    if (strncmp((char *)USART_RX_BUF, "Auto", 4) == 0) {
      mode = AUTO_MODE;
      led_control(D2);
      printf("Mode switched to Auto\r\n");
      update_display(htu21d_t(), htu21d_h());
    }
    // 检查是否接收到"Manual"命令
    else if (strncmp((char *)USART_RX_BUF, "Manual", 6) == 0) {
      mode = MANUAL_MODE;
      led_control(D1);
      printf("Mode switched to Manual\r\n");
      update_display(htu21d_t(), htu21d_h());
    }
    // 清空串口接收缓冲区
    clean_usart();
  }
}

/****************************************
 *          UART通信模块结束            *
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
 *         风扇自动控制模块开始          *
 ****************************************/

void auto_control(float temp, float humi) {
  char new_count;

  // 根据温度范围自动调节风扇档位
  if (temp >= TEMP_HIGH) { // 温度 >= 30°C，高速档
    new_count = 3;
    // 只在最高档时响3下蜂鸣器，和手动模式保持一致
    if (buzzer_count < 3) {
      buzzer_tweet();
      buzzer_count++;
    }
    rgb_ctrl(5);                 // 设置RGB指示灯为高温状态
  } else if (temp >= TEMP_MED) { // 温度 >= 25°C，中速档
    new_count = 2;
    buzzer_count = 0;            // 重置蜂鸣器计数
    rgb_ctrl(4);                 // 设置RGB指示灯为中温状态
  } else if (temp >= TEMP_LOW) { // 温度 >= 20°C，低速档
    new_count = 1;
    buzzer_count = 0; // 重置蜂鸣器计数
    rgb_ctrl(3);      // 设置RGB指示灯为低温状态
  } else {            // 温度 < 20°C，关闭风扇
    new_count = 0;
    buzzer_count = 0; // 重置蜂鸣器计数
    rgb_ctrl(2);      // 设置RGB指示灯为正常状态
  }

  // 如果档位发生变化，更新风扇控制和显示
  if (count != new_count) {
    count = new_count;
    update_display(temp, humi);
  }
}

/****************************************
 *         风扇自动控制模块结束          *
 ****************************************/

void main(void) {
  /****************************************
   *          系统初始化模块开始           *
   ****************************************/

  char buf[30] = {0};
  float current_temp;
  float current_humi;

  // 硬件模块初始化
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
   *        LCD初始化显示模块开始         *
   ****************************************/

  // LCD界面初始化显示 - 显示基本标签和初始值
  sprintf(buf, "Fan PWM:%3d%%", fan_pwm[count]);
  LCDDrawFont16_Next(get_centered_x(buf), 20 + 10 * 7, 4, 320, buf, 0x0000,
                     0xffff); // 显示初始PWM值
  LCDDrawFont16_Next(get_centered_x("Mode:"), 30 + 10 * 8, 4, 320,
                     "Mode:", 0x0000, 0xffff); // 显示模式标签
  LCDDrawFont16_Next(get_centered_x("Temp:"), 30 + 10 * 10, 4, 320,
                     "Temp:", 0x0000, 0xffff); // 显示温度标签
  LCDDrawFont16_Next(get_centered_x("Humi:"), 30 + 10 * 12, 4, 320,
                     "Humi:", 0x0000, 0xffff); // 显示湿度标签

  /****************************************
   *        LCD初始化显示模块结束         *
   ****************************************/

  /****************************************
   *          系统初始化模块结束           *
   ****************************************/

  while (1) {
    // 增加状态输出计时器
    status_output_timer++;

    /****************************************
     *          按键控制模块开始            *
     ****************************************/

    // K4按键：系统电源开关控制
    if (key_interrupt_flag) {
      delay_count(50); // 按键防抖延时
      char current_key = key_value;
      key_interrupt_flag = 0;

      if (current_key == K4_PREESED) {
        system_power = !system_power; // 切换系统电源状态
        buzzer_tweet();               // 按键确认音
        if (system_power == 0) {
          count = 0;                       // 关闭时重置风扇档位
          fan_pwm_control(fan_pwm[count]); // 停止风扇
          rgb_ctrl(2);                     // 设置RGB为正常状态
        }
        update_display(htu21d_t(), htu21d_h());
        continue;
      }
    }

    if (system_power == 0) {
      // 即使系统关闭也要输出状态信息
      if (status_output_timer >=
          1000) { // 每秒输出一次 (假设主循环大约1ms执行一次)
        send_system_status(htu21d_t(), htu21d_h());
        status_output_timer = 0;
      }
      delay_ms(100);
      continue;
    }

    // UART通信检查 - 处理串口接收的模式切换命令
    check_uart_command();
    check_human_presence();

    if (human_present) {
      /****************************************
       *          温湿度监测模块开始           *
       ****************************************/

      // 读取当前温湿度传感器数据
      current_temp = htu21d_t();
      current_humi = htu21d_h();

      /****************************************
       *          温湿度监测模块结束           *
       ****************************************/

      // K3按键：模式切换，K1/K2按键：手动档位调节
      if (key_interrupt_flag) {
        delay_count(50); // 按键防抖延时
        char current_key = key_value;
        key_interrupt_flag = 0;

        if (current_key == K3_PREESED) {
          // K3按键：自动/手动模式切换
          if (mode == AUTO_MODE) {
            mode = MANUAL_MODE; // 切换到手动模式
            led_control(D1);    // 手动模式指示灯
          } else {
            mode = AUTO_MODE; // 切换到自动模式
            led_control(D2);  // 自动模式指示灯
          }
          buzzer_tweet(); // 按键确认音
          update_display(current_temp, current_humi);
        } else if (mode == MANUAL_MODE && current_key == K1_PREESED) {
          // K1按键：手动模式下增加风扇档位
          count++;
          buzzer_tweet(); // 按键确认音
          if (count >= ARRAY(fan_pwm))
            count = ARRAY(fan_pwm) - 1; // 限制最大档位
          update_display(current_temp, current_humi);
        } else if (mode == MANUAL_MODE && current_key == K2_PREESED) {
          // K2按键：手动模式下减少风扇档位
          if (count > 0) {
            count--; // 减少档位
          }
          buzzer_tweet(); // 按键确认音
          update_display(current_temp, current_humi);
        }
      }

      /****************************************
       *          按键控制模块结束            *
       ****************************************/

      // 根据当前模式执行风扇控制策略
      if (mode == AUTO_MODE) {
        // 自动模式：根据温度自动调节风扇转速
        auto_control(current_temp, current_humi);
      } else {
        // 手动模式：根据用户设定的档位控制风扇和指示灯
        if (count == 3) { // 最高档位（100%）
          if (buzzer_count < 3) {
            buzzer_tweet(); // 最高档时蜂鸣器响3下提醒
            buzzer_count++;
          }
          rgb_ctrl(5);           // 高速档RGB指示灯
        } else if (count == 2) { // 中档位（66%）
          buzzer_count = 0;      // 重置蜂鸣器计数
          rgb_ctrl(4);           // 中速档RGB指示灯
        } else if (count == 1) { // 低档位（33%）
          buzzer_count = 0;      // 重置蜂鸣器计数
          rgb_ctrl(3);           // 低速档RGB指示灯
        } else if (count == 0) { // 关闭档位（0%）
          buzzer_count = 0;      // 重置蜂鸣器计数
          rgb_ctrl(2);           // 停止档RGB指示灯
        }
      }

      if (no_motion_timer % 100 == 0) {
        update_display(current_temp, current_humi);
        // send_human_status();
      }

    } else if (mode == AUTO_MODE) {
      // 无人状态下的自动模式：继续根据温度控制风扇
      auto_control(current_temp, current_humi);
      if (no_motion_timer % 100 == 0) {
        update_display(current_temp, current_humi);
        // send_human_status();
      }
    }

    // 每秒输出一次系统状态信息
    if (status_output_timer >= 3) { // 假设主循环大约1ms执行一次
      send_system_status(htu21d_t(), htu21d_h());
      status_output_timer = 0;
    }

    fan_pwm_control(fan_pwm[count]);
    delay_ms(1); // 确保主循环约1ms执行一次
  }
}

/*
else {
    count = 0;
    fan_pwm_control(fan_pwm[count]);
    update_display(current_temp, current_humi);
  }
*/