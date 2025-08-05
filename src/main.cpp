#include <Arduino.h>
#include <FreeRTOS.h>
#include <EEPROM.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <lvgl.h>
#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
#endif
#include <TFT_eSPI.h>
#include "ui/ui.h"
#include "ui/screens.h"
#include "battery.h"
#include "heimann_driver.hpp"
#include "settings_manage.h"
#include "Display.h"
#include "button.h"
#include "shared_val.h"
#include "touchpad.h"
#include "pic_manage.h"
//#include "BilinearInterpolationinpic.h"






/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area));
  tft.pushColors((uint16_t *)px_map, lv_area_get_width(area) * lv_area_get_height(area), true);
  tft.endWrite();
  // tft.setAddrWindow(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area));
  // tft.pushImageDMA(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area), (uint16_t*)px_map, dmaBufferPtr);
  lv_display_flush_ready(disp);
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
  return millis();
}


Button buttonManager;

void setup()
{
  Serial.begin(115200);
  //  delay(1000);
  //  while (!Serial); // 等待串口连接。调试时适用~
  Serial.println("System starting...");
  //lastTime = millis();             // 获取初始时间
  
  set_sys_clock_khz(250000, true);
  Serial.print("CPU frequency: ");
  Serial.print(rp2040.f_cpu() / 1000000.0, 1);
  Serial.println("MHz");

  analogWriteFreq(10000); // 永久设置PWM频率

  lv_init();
  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
  /* register print function for debugging */
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif
  // lv_display_t * disp; 放shared里
#if LV_USE_TFT_ESPI
  /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, TFT_ROTATION);
#else // 手动创建tft。
  /*Else create a display yourseen .lf*/
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  tft.begin();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.initDMA();
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);
  tft.setRotation(1);
#endif

  touch.begin();

  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  Display_init(); // 屏幕亮起。显示LOGO等~
  smooth_off();
  tft.fillScreen(TFT_BLACK);
  ui_init(); // eez UI
  ColorMap_init();
  battery_init();
  initKalmanFilters(3000);
  //rawTempFilter.setNoiseParams(1.2, 1.8); // 原始温度参数
  buttonManager.begin();
  // 设置回调函数
  // buttonManager.setCallbacks(
  //  []() { Serial.println("GPIO24 Short Press Event"); },
  //  []() { Serial.println("GPIO24 Long Press Event"); },
  //  []() { Serial.println("BOOTSEL Short Press Event"); },
  //  []() { Serial.println("BOOTSEL Long Press Event"); },
  //  []() { Serial.println("Combo Short Press Event"); },
  //  []() { Serial.println("Combo Long Press Event"); }
  //);
  load_settings();
  // touch_setup();
  //  让屏幕缓慢亮起添加，，先加载UI。。
  lv_timer_handler(); /* let the GUI do its work */
  ui_tick();          // eez UI
  delay(500);
  smooth_on();
  // 屏幕已亮起。。
  // 等待传感器初始化完成
  // while (!PROB_READY)
  // {
  //   delay(5);
  // }
  touch_gamma_init();

  FATFS_init();
  Serial.println("Setup done");
}

void loop()
{
  delay(5); 
  // Serial.println("Loop start");
  lv_timer_handler(); /* let the GUI do its work */
  ui_tick();          // eez UI
  battery_loop();
  Display_loop();
  buttonManager.update();
  usbFS_loop();
}

//暂留原版兼容配置。
#define MLX_VDD 11

void setup1()
{
  pinMode(MLX_VDD, OUTPUT);
  digitalWrite(MLX_VDD, LOW);

  sensor_setup();
  while (!PROB_READY)
  {
    delay(5);
  }
}
void loop1()
{
  sensor_loop();
  delay(1);
}
