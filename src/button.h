#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <lvgl.h>
#include "shared_val.h"
#include "pic_manage.h"

// 按键时间阈值定义 (毫秒)
#define DEBOUNCE_TIME 30    // 消抖时间
#define SHORT_PRESS_MAX 300 // 短按最大时间
#define LONG_PRESS_MIN 300  // 长按最小时间
#define COMBO_SHORT_MAX 300 // 组合短按最大时间
#define COMBO_LONG_MIN 1000 // 组合长按最小时间
#define COMBO_TOLERANCE 100 // 组合按键时间容差

class Button
{
public:
  // 按钮事件回调函数类型
  typedef void (*ButtonEventCallback)();

  // 构造函数
  Button() {}

  // 初始化GPIO
  void begin()
  {
    pinMode(BUTTON_GPIO, INPUT_PULLUP); // GPIO24上拉输入
    resetStates();
  }

  // 主循环中持续调用
  void update()
  {
    unsigned long currentMillis = millis();

    // 读取按钮状态（GPIO24按下=LOW，BOOTSEL按下=false）
    bool btnGpioActive = (digitalRead(BUTTON_GPIO) == LOW);
    bool btnBootselActive = BOOTSEL;

    // 更新GPIO24按钮状态
    updateButtonState(btnGpioActive, currentMillis,
                      stateGpio, lastGpioChange, pressStartGpio,
                      longPressFiredGpio);

    // 更新BOOTSEL按钮状态
    updateButtonState(btnBootselActive, currentMillis,
                      stateBootsel, lastBootselChange, pressStartBootsel,
                      longPressFiredBootsel);

    // 检查组合按键
    checkComboActivation(currentMillis);

    // 处理组合按键
    if (comboActive)
    {
      handleComboLogic(currentMillis);
    }
  }

  // 设置回调函数
  void setCallbacks(
      ButtonEventCallback gpioShort,
      ButtonEventCallback gpioLong,
      ButtonEventCallback bootselShort,
      ButtonEventCallback bootselLong,
      ButtonEventCallback comboShort,
      ButtonEventCallback comboLong)
  {
    cbGpioShort = gpioShort;
    cbGpioLong = gpioLong;
    cbBootselShort = bootselShort;
    cbBootselLong = bootselLong;
    cbComboShort = comboShort;
    cbComboLong = comboLong;
  }

private:
  // 按钮引脚定义
  static const uint8_t BUTTON_GPIO = 24;

  // 按钮状态枚举
  enum ButtonState
  {
    RELEASED,    // 释放状态
    DEBOUNCING,  // 消抖中
    PRESSED,     // 已按下
    WAIT_RELEASE // 等待释放
  };

  // 单个按钮状态变量
  ButtonState stateGpio = RELEASED;
  ButtonState stateBootsel = RELEASED;
  unsigned long lastGpioChange = 0;
  unsigned long lastBootselChange = 0;
  unsigned long pressStartGpio = 0;
  unsigned long pressStartBootsel = 0;
  bool longPressFiredGpio = false;
  bool longPressFiredBootsel = false;

  // 组合按键状态变量
  bool comboActive = false;
  unsigned long comboStartTime = 0;
  bool comboLongFired = false;

  // 回调函数指针
  ButtonEventCallback cbGpioShort = nullptr;
  ButtonEventCallback cbGpioLong = nullptr;
  ButtonEventCallback cbBootselShort = nullptr;
  ButtonEventCallback cbBootselLong = nullptr;
  ButtonEventCallback cbComboShort = nullptr;
  ButtonEventCallback cbComboLong = nullptr;

  // 重置所有状态
  void resetStates()
  {
    stateGpio = RELEASED;
    stateBootsel = RELEASED;
    comboActive = false;
    comboLongFired = false;
    longPressFiredGpio = false;
    longPressFiredBootsel = false;
    lastGpioChange = lastBootselChange = pressStartGpio = pressStartBootsel = 0;
  }

  // 更新单个按钮状态机
  void updateButtonState(bool active, unsigned long currentMillis,
                         ButtonState &state, unsigned long &lastChange,
                         unsigned long &pressStart, bool &longPressFired)
  {
    switch (state)
    {
    case RELEASED:
      if (active)
      {
        state = DEBOUNCING;
        lastChange = currentMillis;
        longPressFired = false; // 重置长按标志
      }
      break;

    case DEBOUNCING:
      if (!active)
      {
        state = RELEASED;
      }
      else if (currentMillis - lastChange >= DEBOUNCE_TIME)
      {
        state = PRESSED;
        pressStart = currentMillis;
      }
      break;

    case PRESSED:
      if (!active)
      {
        state = WAIT_RELEASE;
        lastChange = currentMillis;
      }
      else
      {
        // 实时检查长按条件
        unsigned long pressDuration = currentMillis - pressStart;
        if (!longPressFired && pressDuration > LONG_PRESS_MIN)
        {
          longPressFired = true;
          if (!comboActive)
          { // 非组合状态下触发长按
            if (&state == &stateGpio)
            {
              triggerEvent(GPIO_LONG);
            }
            else
            {
              triggerEvent(BOOTSEL_LONG);
            }
          }
        }
      }
      break;

    case WAIT_RELEASE:
      if (active)
      {
        state = PRESSED;
      }
      else if (currentMillis - lastChange >= DEBOUNCE_TIME)
      {
        state = RELEASED;
        // 处理短按事件
        unsigned long pressDuration = currentMillis - pressStart;
        if (!longPressFired && pressDuration <= SHORT_PRESS_MAX && !comboActive)
        {
          if (&state == &stateGpio)
          {
            triggerEvent(GPIO_SHORT);
          }
          else
          {
            triggerEvent(BOOTSEL_SHORT);
          }
        }
        pressStart = 0; // 重置按压开始时间
      }
      break;
    }
  }

  // 检查并激活组合按键
  void checkComboActivation(unsigned long currentMillis)
  {
    if (!comboActive && stateGpio == PRESSED && stateBootsel == PRESSED)
    {
      unsigned long pressTimeGpio = currentMillis - pressStartGpio;
      unsigned long pressTimeBootsel = currentMillis - pressStartBootsel;

      // 计算时间差（避免使用abs）
      unsigned long diff = (pressTimeGpio > pressTimeBootsel)
                               ? (pressTimeGpio - pressTimeBootsel)
                               : (pressTimeBootsel - pressTimeGpio);

      if (diff <= COMBO_TOLERANCE)
      {
        comboActive = true;
        comboStartTime = currentMillis;
        comboLongFired = false;
        // 标记已触发长按，防止单独触发
        longPressFiredGpio = true;
        longPressFiredBootsel = true;
      }
    }
  }

  // 处理组合按键逻辑
  void handleComboLogic(unsigned long currentMillis)
  {
    bool bothPressed = (stateGpio == PRESSED && stateBootsel == PRESSED);
    unsigned long comboDuration = currentMillis - comboStartTime;

    // 长按检测（立即触发）
    if (!comboLongFired && comboDuration >= COMBO_LONG_MIN)
    {
      comboLongFired = true;
      triggerEvent(COMBO_LONG);
    }

    // 按键释放处理
    if (!bothPressed)
    {
      // 检查是否两个按钮都已释放
      if (stateGpio != PRESSED && stateBootsel != PRESSED)
      {
        // 短按检测
        if (comboDuration <= COMBO_SHORT_MAX && !comboLongFired)
        {
          triggerEvent(COMBO_SHORT);
        }
        comboActive = false;
      }
    }
  }





  void BOOTSEL_Short()
  {
    if (in_settings)
    {
    }
    else
    {
      next_cmap();
    }
  }
  void GPIO24_Short()
  {
    if (in_settings)
    {
    }
    else
    {
      flag_in_photo_mode = !flag_in_photo_mode;
      if(flag_in_photo_mode)
      {
        Serial.println("进入拍照模式");
        save_data_pixel();
      }
    }
  }

  void BOOTSEL_Long()
  {
    Serial.println("BOOTSEL Long Press Detected");
    if (in_settings)
    {
    }
    else
    {
      in_settings = true;
      loadScreen(SCREEN_ID_SETTINGS);
    }
  }

  
  void GPIO24_Long()
  {
    Serial.println("GPIO24 Long Press Detected");
    if (in_settings)
    {
      toMainScreen();
    }
    else
    {

    }
  }

  // 触发事件处理
  void triggerEvent(uint8_t eventType)
  {
    switch (eventType)
    {
    case GPIO_SHORT:
      if (cbGpioShort)
        cbGpioShort();
      Serial.println("GPIO24 Short Press");
      GPIO24_Short();
      break;

    case GPIO_LONG:
      if (cbGpioLong)
        cbGpioLong();
      Serial.println("GPIO24 Long Press");
      GPIO24_Long();
      break;

    case BOOTSEL_SHORT:
      if (cbBootselShort)
        cbBootselShort();
      Serial.println("BOOTSEL Short Press");
      BOOTSEL_Short();
      break;

    case BOOTSEL_LONG:
      if (cbBootselLong)
        cbBootselLong();
      Serial.println("BOOTSEL Long Press");
      BOOTSEL_Long();
      break;

    case COMBO_SHORT:
      if (cbComboShort)
        cbComboShort();
      Serial.println("Combo Short Press");
      break;

    case COMBO_LONG:
      if (cbComboLong)
        cbComboLong();
      Serial.println("Combo Long Press");
      break;
    }
  }

  // 事件类型枚举
  enum EventType
  {
    GPIO_SHORT,
    GPIO_LONG,
    BOOTSEL_SHORT,
    BOOTSEL_LONG,
    COMBO_SHORT,
    COMBO_LONG
  };
};

#endif // BUTTON_H