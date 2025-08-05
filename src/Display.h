#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <FreeRTOS.h>
#include "shared_val.h"
#include "heimann_driver.hpp"
#include "BilinearInterpolation.h"
#include "colormap.h"
#include <lvgl.h>
#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
#endif
#include <font/lv_font.h>
#include "kalman_raw_temp.h"
#include "kalman_mapped_temp.h"
#include "logo_jpg.h"
#include <TJpg_Decoder.h>
// 全局滤波器实例
//KalmanFilterRawUShort rawTempFilter;        // 用于原始温度
KalmanFilterMappedUShort mappedValueFilter; // 用于映射值

#define PROB_SCALE 7.5
const int CANVAS_WIDTH = 240;
const int CANVAS_HEIGHT = 240;

float ft_point; // 屏幕光标点的温度值

void smooth_on()
{
  analogWrite(SCREEN_BL_PIN, 0);
  for (int i = 0; i < brightness; i++)
  {
    analogWrite(SCREEN_BL_PIN, i);
    delay(2);
  }
}

void smooth_off()
{
  analogWrite(SCREEN_BL_PIN, brightness);
  for (int i = brightness; i > 3; i--)
  {
    analogWrite(SCREEN_BL_PIN, i);
    delay(2);
  }
}

// 背光调节,会限制输入亮度在正确范围内
void set_brightness(int _brightness)
{
  // 使用约束函数确保亮度在有效范围内 [5, 255]
  int clamped_brightness = constrain(_brightness, 5, 255);

  // 设置亮度
  analogWrite(SCREEN_BL_PIN, clamped_brightness);
  brightness = clamped_brightness;
}

uint16_t *logoBuffer1 = nullptr; // Toggle buffer for 16*16 MCU block
uint16_t *logoBuffer2 = nullptr; // Toggle buffer for 16*16 MCU block
uint16_t *logoBufferPtr = logoBuffer1;
bool logoBufferSel = 0;
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if (y >= tft.height())
    return 0;
  if (logoBufferSel)
    logoBufferPtr = logoBuffer2;
  else
    logoBufferPtr = logoBuffer1;
  logoBufferSel = !logoBufferSel; // Toggle buffer selection
  //  pushImageDMA() will clip the image block at screen boundaries before initiating DMA
  tft.pushImageDMA(x, y, w, h, bitmap, logoBufferPtr); // Initiate DMA - blocking only if last DMA is not complete
  // The DMA transfer of image block to the TFT is now in progress...
  // Return 1 to decode next block.
  return 1;
}

void render_logo()
{
  // 动态分配内存
  logoBuffer1 = (uint16_t *)malloc(16 * 16 * sizeof(uint16_t));
  logoBuffer2 = (uint16_t *)malloc(16 * 16 * sizeof(uint16_t));

  if (logoBuffer1 == nullptr || logoBuffer2 == nullptr)
  {
    // 内存分配失败，处理错误
    if (logoBuffer1)
      free(logoBuffer1);
    if (logoBuffer2)
      free(logoBuffer2);
    return;
  }
  tft.fillScreen(TFT_BLACK);
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tft_output);
  uint16_t w = 0, h = 0;
  TJpgDec.getJpgSize(&w, &h, logo, sizeof(logo));
  tft.startWrite();
  TJpgDec.drawJpg(0, 0, logo, sizeof(logo));
  tft.endWrite();
  // 释放内存
  free(logoBuffer1);
  free(logoBuffer2);
  logoBuffer1 = nullptr;
  logoBuffer2 = nullptr;
}

void Display_init()
{
  Serial.println("Display_init starting...");
  // 背光开关
  pinMode(SCREEN_BL_PIN, OUTPUT);
  digitalWrite(SCREEN_BL_PIN, LOW);
  // 屏幕开关
  pinMode(SCREEN_VDD, OUTPUT);
  digitalWrite(SCREEN_VDD, LOW);
  // 先读取图片 屏幕再亮起
  render_logo();
  delay(300);
  smooth_on();
  delay(1000); // 显示1秒
  Serial.println("Display_init initialized");
}

// 全局颜色常量 避免重复计算
static const uint16_t CROSS_WHITE = tft.color565(255, 255, 255);
static const uint16_t CROSS_BLACK = tft.color565(0, 0, 0);

// 优化后的点测温显示函数
inline void draw_cross_and_temp(float temp, int x, int y, int draw_line = -1)
{
  if (draw_line != -1)
  {
    // 计算触发行位置（光标位置上方40行）
    int trigger_line = test_point[1] + 20;

    // 确保触发行在有效范围内
    if (trigger_line > 215)
      trigger_line = 215;

    // 检查是否需要绘制
    if (draw_line != trigger_line || !flag_show_cursor)
      return;
  }

  // 预计算常用值
  const int half_len = cursor_size / 2;
  const int quarter_len = cursor_size / 4;

  // 合并水平/垂直线绘制
  tft.drawLine(x - half_len, y, x + half_len, y, CROSS_WHITE); // 水平白线
  tft.drawLine(x, y - half_len, x, y + half_len, CROSS_WHITE); // 垂直白线

  tft.drawLine(x - quarter_len, y, x + quarter_len, y, CROSS_BLACK); // 水平黑线
  tft.drawLine(x, y - quarter_len, x, y + quarter_len, CROSS_BLACK); // 垂直黑线

  if (!flag_show_temp_text)
    return; // 不显示温度直接返回

  // 根据位置智能计算文本偏移量
  const int shift_x = (x < 140) ? 10 : -60;
  const int shift_y = (y < 120) ? 10 : -20;

  // 创建格式化字符串（避免浮点格式化开销）
  char temp_str[8];
  const int len = snprintf(temp_str, sizeof(temp_str), "%.2f", temp);

  // 设置文本属性并显示
  tft.setTextSize(temp_text_size);
  tft.setCursor(x + shift_x, y + shift_y);
  tft.print(temp_str);
}

const int lines = 3;
uint16_t lineBuffer[CANVAS_WIDTH * lines];
uint16_t dmaBuffer1[CANVAS_WIDTH * lines];
uint16_t dmaBuffer2[CANVAS_WIDTH * lines];
uint16_t *dmaBufferPtr = dmaBuffer1;
bool dmaBufferSel = false;

//CallFrequency counter1 = initFrequencyCounter("DrawFunction");

// 热成像图像的显示
void Draw()
{
  //updateFrequency(counter1);
  tft.startWrite();
  for (int y_start = 0; y_start < CANVAS_HEIGHT; y_start += lines)
  {
    // 计算当前块实际行数
    const int chunk_lines = (y_start + lines <= CANVAS_HEIGHT)
                                ? lines
                                : CANVAS_HEIGHT - y_start;
    // 填充行缓冲区
    for (int y_offset = 0; y_offset < chunk_lines; y_offset++)
    {
      const int y = y_start + y_offset;
      uint16_t *line = &lineBuffer[y_offset * CANVAS_WIDTH];

      for (int x = 0; x < CANVAS_WIDTH; x++)
      {
        int value = bio_interpolate(RES_240x240,interpolation_method, y, x, draw_pixel);
        line[x] = colormap[value];
      }
      draw_cross_and_temp(ft_point, test_point[0], test_point[1], y);
    }
    // 切换DMA缓冲区
    dmaBufferPtr = dmaBufferSel ? dmaBuffer2 : dmaBuffer1;
    dmaBufferSel = !dmaBufferSel;

    // DMA传输图像块
    tft.pushImageDMA(0, y_start, CANVAS_WIDTH, chunk_lines, lineBuffer, dmaBufferPtr);
  }
  tft.endWrite();
}

static char label_buf[8];
constexpr float DIVISOR = 0.1f;
constexpr float KELVIN_OFFSET = 273.15f;


void data_pixel_to_draw_pixel_in_pic(unsigned short data[GRID_SIZE][GRID_SIZE], unsigned short temp_T_max, unsigned short temp_T_min)
{
  // 卡尔曼滤波——原始数据
  // if (use_kalman_model >= 1)
  // {
  //   rawTempFilter.updateTempRange(temp_T_min, temp_T_max);
  //   rawTempFilter.processFrame(new_data_pixel);
  // }

  unsigned short value;
  for (int i = 0; i < 32; i++)
  {
    for (int j = 0; j < 32; j++)
    {
      //Serial.println(new_data_pixel[i][j]);
      // 拷贝温度信息, 并提前映射到色彩空间中
      value = (color_num * (new_data_pixel[i][j] - temp_T_min) / (temp_T_max - temp_T_min));
      if (value < color_num)
      {
        draw_pixel[i][j] = value;
      }
    }
  }


  // if (use_kalman_model >= 2)
  // {
  //   // 应用映射值卡尔曼滤波
  //   mappedValueFilter.processFrame(draw_pixel);
  // }
}



void data_pixel_to_draw_pixel(unsigned short data[GRID_SIZE][GRID_SIZE])
{

  // 卡尔曼滤波——原始数据
  if (use_kalman_model >= 1)
  {
    //rawTempFilter.updateTempRange(T_min, T_max);
    //rawTempFilter.processFrame(new_data_pixel);
    processFrame(new_data_pixel);
  }

  // 预先计算常量值
  const int range = T_max - T_min;
  const float scale_factor = (range != 0) ? (color_num_f / range) : 0;

  // 使用指针
  uint16_t *pixel_ptr = reinterpret_cast<uint16_t *>(&draw_pixel[0][0]);
  const uint16_t *data_ptr = &new_data_pixel[0][0];

  // 循环展开处理4个元素/迭代
  for (int i = 0; i < 1024; i += 4)
  { // 32*32=1024
    // 无分支计算 - 使用条件移动代替分支
    float val0 = (static_cast<int>(data_ptr[i]) - T_min) * scale_factor;
    float val1 = (static_cast<int>(data_ptr[i + 1]) - T_min) * scale_factor;
    float val2 = (static_cast<int>(data_ptr[i + 2]) - T_min) * scale_factor;
    float val3 = (static_cast<int>(data_ptr[i + 3]) - T_min) * scale_factor;

    // 饱和处理并转换类型
    pixel_ptr[i] = static_cast<uint16_t>((val0 < color_num_f) ? val0 : color_num_f);
    pixel_ptr[i + 1] = static_cast<uint16_t>((val1 < color_num_f) ? val1 : color_num_f);
    pixel_ptr[i + 2] = static_cast<uint16_t>((val2 < color_num_f) ? val2 : color_num_f);
    pixel_ptr[i + 3] = static_cast<uint16_t>((val3 < color_num_f) ? val3 : color_num_f);
  }
  // for (int i = 0; i < 32; i++)
  // {
  //   for (int j = 0; j < 32; j++)
  //   {
  //     // 拷贝温度信息, 并提前映射到色彩空间中
  //     value = (180 * (new_data_pixel[i][j] - T_min) / (T_max - T_min));
  //     if (value < 180)
  //     {
  //       draw_pixel[i][j] = value;
  //     }
  //   }
  // }

  if (use_kalman_model >= 2)
  {
    // 应用映射值卡尔曼滤波
    mappedValueFilter.processFrame(draw_pixel);
  }






}


void Display_loop()
{
  if (in_settings)
  {
    // 在设置界面时不进行温度采集和显示
    return;
  }
  if (flag_in_photo_mode)
  { // 拍照模式，暂停模式
  }
  else
  { // 正常模式
    float ft_max = static_cast<float>(T_max) * DIVISOR - KELVIN_OFFSET;
    float ft_min = static_cast<float>(T_min) * DIVISOR - KELVIN_OFFSET;
    // float ft_avg = static_cast<float>(T_avg) * DIVISOR - KELVIN_OFFSET;

    unsigned short value;
    waitForUnlock(prob_lock, 5);

    {
      LockGuard lock(pix_cp_lock);                                                                                            // 线程锁的域
      process_rotation_to_new(data_pixel, new_data_pixel, ROTATION_90_CW_TO_NEW);                                             // 把旋转后的新数据存入新数组
      ft_point = (float)(new_data_pixel[(int)(test_point[0] / PROB_SCALE)][(int)(test_point[1] / PROB_SCALE)] / 10) - 273.15; // 中心点/选择点 温度
      data_pixel_to_draw_pixel(new_data_pixel); // 将新数据转换为绘图数据

    }

    waitForUnlock(cmap_loading_lock, 1);
    
    Draw();



    // 写最大/最小/平均/电量/模式
    snprintf(label_buf, sizeof(label_buf), "%5.1f", ft_max);
    lv_label_set_text(objects.t_1, label_buf);
    // lv_label_set_text_fmt(objects.t_1, "%5.1f", ft_max);

    snprintf(label_buf, sizeof(label_buf), "%5.1f", ft_min);
    lv_label_set_text(objects.t_2, label_buf);
    // lv_label_set_text_fmt(objects.t_2, "%5.1f", ft_min);

    // snprintf(label_buf, sizeof(label_buf), "%d", epsilon);
    // lv_label_set_text(objects.t_3, label_buf);
    lv_label_set_text_fmt(objects.t_3, "%d", epsilon);

    // snprintf(label_buf, sizeof(label_buf), "%s", ColorMapNames[cmap_now_choose]);
    // lv_label_set_text(objects.t_4, label_buf);
    lv_label_set_text_fmt(objects.t_4, "%s", get_current_colormap_name());

    snprintf(label_buf, sizeof(label_buf), "%5.1f", ft_point);
    lv_label_set_text(objects.t_5, label_buf);
    // lv_label_set_text_fmt(objects.t_5, "%5.1f", ft_point);

    snprintf(label_buf, sizeof(label_buf), "%5.1f", current_gamma);
    lv_label_set_text(objects.label_gamma, label_buf);



    if (flag_show_cursor)
      draw_cross_and_temp(ft_point, test_point[0], test_point[1], -1);

    if (flag_trace_max)
      draw_cross_and_temp(ft_max, (x_max * 15 + 1) >> 1, ((31 - y_max) * 15 + 1) >> 1, -1);

    if (flag_trace_min)
      draw_cross_and_temp(ft_max, (x_min * 15 + 1) >> 1, ((31 - y_min) * 15 + 1) >> 1, -1);
  }
}

#endif