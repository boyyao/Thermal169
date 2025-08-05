#ifndef COLORMAP_H
#define COLORMAP_H
#include <lvgl.h>
#include <Arduino.h>
#include "shared_val.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include "lvgl.h"
static uint16_t colormap[color_num];

typedef struct
{
  float pos;
  uint8_t r, g, b;
} ColorKeypoint;

// 预设
typedef enum
{
  IRON = 0,
  RAINBOW,
  COLD,
  HOT_METAL,
  MEDICAL,
  FIRE,
  ICE_FIRE,
  BLACK_HOT,
  WHITE_HOT,
  HIGH_CONTRAST,
  OCEAN,
  VIRIDIS,
  AMBER,
  NIGHT_VISION,
  RAINBOW2,
  COPPER,
  PLASMA,
  INFERNO,
  MAGMA,
  GREEN_RED_BLUE,
  SUNSET,
  AQUA,
  FOREST,
  GOLD,
  LAVA,
  NEON,
  PASTEL,
  SEPIA,
  STARRY_NIGHT,
  VOLCANO,
  VIRIDIS2,
  CLASSIC2,
  HOT2,
  RAINBOW3,
  INFERNO2,
  GREYS_R
} ColormapPreset;


#define COLORMAP_PRESET_COUNT 30

// 全局共享的伽马LUT
static uint8_t shared_gamma_lut_r[32];
static uint8_t shared_gamma_lut_g[64];
static uint8_t shared_gamma_lut_b[32];
static float current_shared_gamma = 0.0f;

// 伽马值
static const float preset_gamma[COLORMAP_PRESET_COUNT] = {
    1.8f, // IRON
    2.2f, // RAINBOW
    1.6f, // COLD
    1.7f, // HOT_METAL
    2.0f, // MEDICAL
    2.4f, // FIRE
    0.7f, // ICE_FIRE yyyyyyyyy
    1.5f, // BLACK_HOT
    1.5f, // WHITE_HOT
    2.6f, // HIGH_CONTRASTxxxxxxxx
    1.8f, // OCEAN
    2.0f, // VIRIDIS
    1.9f, // AMBER
    2.0f, // NIGHT_VISION
    1.0f, // RAINBOW2 yyyyyyyyy
    1.8f, // COPPER
    2.0f, // PLASMA
    2.2f, // INFERNO
    2.1f, // MAGMA
    2.0f, // GREEN_RED_BLUE
    2.0f, // SUNSET
    1.7f, // AQUA
    1.8f, // FOREST
    1.9f, // GOLD
    2.3f, // LAVA
    2.5f, // NEON
    1.6f, // PASTEL
    1.7f, // SEPIA
    1.8f, // STARRY_NIGHT
    2.4f, // VOLCANO
    };

// 名称
static const char *colormap_names[COLORMAP_PRESET_COUNT] = {
    "IRON",
    "RAINBOW",
    "COLD",
    "HOT_METAL",
    "MEDICAL",
    "FIRE",
    "ICE_FIRE",
    "BLACK_HOT",
    "WHITE_HOT",
    "HIGH_CONTRAST",
    "OCEAN",
    "VIRIDIS",
    "AMBER",
    "NIGHT_VISION",
    "RAINBOW2",
    "COPPER",
    "PLASMA",
    "INFERNO",
    "MAGMA",
    "GREEN_RED_BLUE",
    "SUNSET",
    "AQUA",
    "FOREST",
    "GOLD",
    "LAVA",
    "NEON",
    "PASTEL",
    "SEPIA",
    "STARRY_NIGHT",
    "VOLCANO"};

// 伽马
void init_shared_gamma_lut(float gamma)
{
  if (gamma < 0.01f)
  {
    Serial.println("[WARNING] Gamma < 0.01 clamped to 0.01");
    gamma = 0.01f;
  }
  else if (gamma > 20.0f)
  {
    Serial.println("[WARNING] Gamma > 20.0 clamped to 20.0");
    gamma = 20.0f;
  }

  if (fabsf(gamma - current_shared_gamma) > 0.005f)
  {
    for (int i = 0; i < 32; i++)
    {
      float norm = i / 31.0f;
      // 防止NaN
      if (norm < FLT_MIN)
        norm = FLT_MIN;
      shared_gamma_lut_r[i] = (uint8_t)(powf(norm, gamma) * 31 + 0.5f);
      shared_gamma_lut_b[i] = shared_gamma_lut_r[i];
    }
    for (int i = 0; i < 64; i++)
    {
      float norm = i / 63.0f;
      if (norm < FLT_MIN)
        norm = FLT_MIN;
      shared_gamma_lut_g[i] = (uint8_t)(powf(norm, gamma) * 63 + 0.5f);
    }
    current_shared_gamma = gamma;

    // 打印更新信息
    char msg[64];
    snprintf(msg, sizeof(msg), "[Gamma] Updated shared LUT: %.3f", gamma);
    Serial.println(msg);
  }
}
// 预设
const ColorKeypoint *get_preset_keypoints(ColormapPreset preset, int *num_keys)
{
  static const ColorKeypoint presets[][7] = {
      // 铁红色 (IRON) - 4点
      {{0.0f, 0, 0, 0}, {0.3f, 64, 0, 0}, {0.6f, 192, 64, 0}, {1.0f, 255, 192, 64}},

      // 彩虹 (RAINBOW) - 5点
      {{0.0f, 0, 0, 255}, {0.25f, 0, 255, 255}, {0.5f, 0, 255, 0}, {0.75f, 255, 255, 0}, {1.0f, 255, 0, 0}},

      // 冷色调 (COLD) - 4点
      {{0.0f, 0, 0, 128}, {0.3f, 0, 64, 255}, {0.7f, 64, 192, 255}, {1.0f, 192, 255, 255}},

      // 热金属 (HOT_METAL) - 5点
      {{0.0f, 0, 0, 0}, {0.2f, 128, 0, 0}, {0.5f, 255, 128, 0}, {0.8f, 255, 255, 128}, {1.0f, 255, 255, 255}},

      // 医疗 (MEDICAL) - 5点
      {{0.0f, 0, 0, 0}, {0.2f, 0, 0, 128}, {0.5f, 0, 192, 0}, {0.8f, 255, 255, 0}, {1.0f, 255, 128, 128}},

      // 火焰 (FIRE) - 5点
      {{0.0f, 0, 0, 0}, {0.3f, 128, 0, 0}, {0.6f, 255, 128, 0}, {0.9f, 255, 255, 0}, {1.0f, 255, 255, 255}},

      // 冰火 (ICE_FIRE) - 5点 0.5-0.7OK
      {{0.0f, 0, 0, 128}, {0.3f, 0, 128, 255}, {0.5f, 0, 0, 0}, {0.7f, 255, 128, 0}, {1.0f, 255, 255, 192}},

      // 黑热 (BLACK_HOT) - 4点
      {{0.0f, 255, 255, 255}, {0.3f, 128, 128, 128}, {0.7f, 64, 64, 64}, {1.0f, 0, 0, 0}},

      // 白热 (WHITE_HOT) - 4点
      {{0.0f, 0, 0, 0}, {0.3f, 64, 64, 64}, {0.7f, 128, 128, 128}, {1.0f, 255, 255, 255}},

      // 高对比度 (HIGH_CONTRAST) - 4点
      {{0.0f, 0, 0, 255}, {0.4f, 255, 0, 0}, {0.6f, 255, 0, 0}, {1.0f, 255, 255, 0}},

      // 海洋蓝调 (OCEAN) - 4点
      {{0.0f, 0, 0, 64}, {0.3f, 0, 64, 128}, {0.6f, 0, 128, 192}, {1.0f, 0, 192, 255}},

      // 科学可视化 (VIRIDIS) - 5点
      {{0.0f, 68, 1, 84}, {0.25f, 59, 82, 139}, {0.5f, 33, 145, 140}, {0.75f, 94, 201, 98}, {1.0f, 253, 231, 37}},

      // 琥珀色 (AMBER) - 5点
      {{0.0f, 0, 0, 0}, {0.2f, 64, 32, 0}, {0.5f, 192, 96, 0}, {0.8f, 255, 192, 64}, {1.0f, 255, 255, 192}},

      // 夜视绿 (NIGHT_VISION) - 5点
      {{0.0f, 0, 0, 0}, {0.1f, 0, 8, 0}, {0.4f, 0, 64, 0}, {0.8f, 32, 192, 32}, {1.0f, 192, 255, 192}},

      // 增强彩虹 (RAINBOW2) - 7点
      {{0.0f, 0, 0, 128}, {0.2f, 0, 0, 255}, {0.4f, 0, 128, 255}, {0.6f, 0, 255, 128}, {0.7f, 255, 255, 0}, {0.9f, 255, 128, 0}, {1.0f, 255, 0, 0}},

      // 铜色调 (COPPER) - 5点
      {{0.0f, 0, 0, 0}, {0.2f, 64, 32, 0}, {0.5f, 160, 80, 0}, {0.8f, 220, 160, 32}, {1.0f, 255, 220, 128}},

      // 等离子体 (PLASMA) - 5点
      {{0.0f, 13, 8, 135}, {0.25f, 75, 3, 161}, {0.5f, 190, 23, 115}, {0.75f, 240, 90, 32}, {1.0f, 248, 220, 20}},

      // 地狱火 (INFERNO) - 5点
      {{0.0f, 0, 0, 4}, {0.25f, 60, 5, 80}, {0.5f, 150, 30, 15}, {0.75f, 230, 100, 10}, {1.0f, 252, 255, 164}},

      // 岩浆 (MAGMA) - 5点
      {{0.0f, 0, 0, 4}, {0.25f, 60, 15, 110}, {0.5f, 140, 30, 150}, {0.75f, 210, 80, 100}, {1.0f, 252, 220, 200}},

      // 绿-红-蓝 (GREEN_RED_BLUE) - 5点
      {{0.0f, 0, 128, 0}, {0.25f, 0, 255, 0}, {0.5f, 255, 255, 0}, {0.75f, 255, 0, 0}, {1.0f, 0, 0, 255}},

      // 日落 (SUNSET) - 5点
      {{0.0f, 40, 10, 100}, {0.25f, 120, 50, 150}, {0.5f, 220, 100, 50}, {0.75f, 240, 180, 30}, {1.0f, 255, 220, 180}},

      // 水色 (AQUA) - 4点
      {{0.0f, 10, 20, 50}, {0.3f, 50, 100, 150}, {0.7f, 100, 200, 220}, {1.0f, 180, 240, 255}},

      // 森林 (FOREST) - 5点
      {{0.0f, 0, 20, 0}, {0.25f, 0, 80, 0}, {0.5f, 50, 120, 30}, {0.75f, 150, 180, 50}, {1.0f, 220, 255, 180}},

      // 金色 (GOLD) - 4点
      {{0.0f, 20, 10, 0}, {0.3f, 120, 80, 0}, {0.7f, 200, 160, 30}, {1.0f, 255, 220, 100}},

      // 熔岩 (LAVA) - 5点
      {{0.0f, 0, 0, 0}, {0.2f, 80, 0, 0}, {0.5f, 200, 60, 0}, {0.8f, 255, 150, 30}, {1.0f, 255, 255, 180}},

      // 霓虹 (NEON) - 6点
      {{0.0f, 0, 0, 0}, {0.2f, 0, 0, 255}, {0.4f, 0, 255, 255}, {0.6f, 0, 255, 0}, {0.8f, 255, 255, 0}, {1.0f, 255, 0, 255}},

      // 柔和 (PASTEL) - 5点
      {{0.0f, 150, 200, 255}, {0.25f, 200, 150, 255}, {0.5f, 255, 200, 150}, {0.75f, 255, 255, 200}, {1.0f, 200, 255, 200}},

      // 棕褐色 (SEPIA) - 4点
      {{0.0f, 30, 20, 10}, {0.3f, 100, 70, 40}, {0.7f, 180, 140, 90}, {1.0f, 240, 220, 180}},

      // 星空 (STARRY_NIGHT) - 5点
      {{0.0f, 0, 0, 30}, {0.2f, 10, 10, 80}, {0.5f, 50, 30, 150}, {0.8f, 150, 100, 200}, {1.0f, 220, 220, 255}},

      // 火山 (VOLCANO) - 5点
      {{0.0f, 0, 0, 0}, {0.2f, 80, 0, 0}, {0.4f, 180, 40, 0}, {0.7f, 220, 100, 20}, {1.0f, 255, 200, 100}},

  };

  // 设置每种预设的关键点数量
  static const int key_counts[COLORMAP_PRESET_COUNT] = {
      4, 5, 4, 5, 5, // IRON, RAINBOW, COLD, HOT_METAL, MEDICAL
      5, 5, 4, 4, 4, // FIRE, ICE_FIRE, BLACK_HOT, WHITE_HOT, HIGH_CONTRAST
      4, 5, 5, 5, 7, // OCEAN, VIRIDIS, AMBER, NIGHT_VISION, RAINBOW2
      5, 5, 5, 5, 5, // COPPER, PLASMA, INFERNO, MAGMA, GREEN_RED_BLUE
      5, 4, 5, 4, 5, // SUNSET, AQUA, FOREST, GOLD, LAVA
      6, 5, 4, 5, 5, // NEON, PASTEL, SEPIA, STARRY_NIGHT, VOLCANO
      };

  *num_keys = key_counts[preset];
  return presets[preset];
}

// 获取预设的推荐伽马值
float get_preset_gamma(ColormapPreset preset)
{
  if (preset >= 0 && preset < COLORMAP_PRESET_COUNT)
  {
    return preset_gamma[preset];
  }
  return 2.2f; // 默认值
}

// 获取调色板名称
const char *get_colormap_name(ColormapPreset preset)
{
  if (preset >= 0 && preset < COLORMAP_PRESET_COUNT)
  {
    return colormap_names[preset];
  }
  return "UNKNOWN";
}

void generate_colormap(uint16_t colormap[color_num], ColormapPreset preset, float gamma)
{
  init_shared_gamma_lut(gamma);

  int num_keys;
  const ColorKeypoint *keys = get_preset_keypoints(preset, &num_keys);

  for (int i = 0; i < color_num; i++)
  {
    float pos = i / (color_num_f - 1);
    int start_idx = 0, end_idx = num_keys - 1;
    for (int k = 0; k < num_keys - 1; k++)
    {
      if (pos >= keys[k].pos && pos <= keys[k + 1].pos)
      {
        start_idx = k;
        end_idx = k + 1;
        break;
      }
    }
    float segment_length = keys[end_idx].pos - keys[start_idx].pos;
    float t = (segment_length > 0.0001f) ? (pos - keys[start_idx].pos) / segment_length : 0.0f;

    // 平滑
    t = t * t * (3.0f - 2.0f * t);
    uint8_t r = (uint8_t)(keys[start_idx].r + t * (keys[end_idx].r - keys[start_idx].r));
    uint8_t g = (uint8_t)(keys[start_idx].g + t * (keys[end_idx].g - keys[start_idx].g));
    uint8_t b = (uint8_t)(keys[start_idx].b + t * (keys[end_idx].b - keys[start_idx].b));

    uint8_t r5 = shared_gamma_lut_r[r >> 3]; 
    uint8_t g6 = shared_gamma_lut_g[g >> 2]; 
    uint8_t b5 = shared_gamma_lut_b[b >> 3]; 

    colormap[i] = (r5 << 11) | (g6 << 5) | b5;
  }
}

ColormapPreset current_preset = RAINBOW2;
float current_gamma = 2.2f;
bool use_preset_gamma = true; // 是否使用预设伽马值

static LV_ATTRIBUTE_MEM_ALIGN lv_color_t canvas2_buf[2 * 240] __attribute__((aligned(4)));
void ColorMap_init()
{
  lv_canvas_set_buffer(objects.can_2, canvas2_buf, 2, 240, LV_COLOR_FORMAT_RGB565);
}

// 垂直
void showCan_2()
{
  uint16_t *buf_raw2 = reinterpret_cast<uint16_t *>(canvas2_buf);
  uint16_t row_colors[240];
  for (int y = 0; y < 240; y++)
  {
    uint16_t color_index = (y * (color_num - 1)) / 239; 
    if (color_index >= color_num)
      color_index = color_num - 1;
    row_colors[y] = colormap[color_index];
  }
  // 竖向填充：每行写入2个相同颜色的像素
  for (int y = 0; y < 240; y++)
  {
    buf_raw2[y * 2] = row_colors[y];     // 行首像素
    buf_raw2[y * 2 + 1] = row_colors[y]; // 行尾像素
  }
  lv_obj_invalidate(objects.can_2);
}

// 加载当前调色板
void load_colormap()
{
  // 如果使用预设伽马值，则获取当前预设的推荐值
  float gamma = use_preset_gamma ? get_preset_gamma(current_preset) : current_gamma;
  {
    LockGuard lock(cmap_loading_lock);
    generate_colormap(colormap, current_preset, gamma);
  }
  current_gamma = gamma;

  showCan_2();
  // 打印调色板信息
  char msg[128];
  snprintf(msg, sizeof(msg), "Loaded colormap: %s, Gamma: %.2f (%s)",
           get_colormap_name(current_preset),
           gamma,
           use_preset_gamma ? "preset" : "custom");
  Serial.println(msg);
}

void load_colormap(uint8_t index)
{
  current_preset = (ColormapPreset)index;
  load_colormap();
}

// 获取当前调色板名称
const char *get_current_colormap_name()
{
  return get_colormap_name(current_preset);
}

// 下一个调色板方案
void next_cmap()
{
  int next = (int)current_preset + 1;
  if (next >= COLORMAP_PRESET_COUNT)
    next = 0;
  current_preset = (ColormapPreset)next;

  use_preset_gamma = true;
  load_colormap();
}

// 上一个调色板方案
void prev_cmap()
{
  int prev = (int)current_preset - 1;
  if (prev < 0)
    prev = COLORMAP_PRESET_COUNT - 1;
  current_preset = (ColormapPreset)prev;

  use_preset_gamma = true;
  load_colormap();
}
// 设置伽马值
void set_gamma(float gamma)
{
  char msg[64];
  snprintf(msg, sizeof(msg), "[Gamma] Set custom: %.3f", gamma);
  Serial.println(msg);

  current_gamma = gamma;
  use_preset_gamma = false;
  load_colormap();
}

// 增加伽马值
void increase_gamma(float step = 0.1f)
{
  set_gamma(current_gamma + step);
}

// 减少伽马值
void decrease_gamma(float step = 0.1f)
{
  set_gamma(current_gamma - step);
}

static const float GAMMA_MIN = 0.1f;
static const float GAMMA_MAX = 10.0f;
static const float SENSITIVITY = 0.01f; // 每像素调整量

// 伽马控制回调函数
static void gamma_ctrl_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_indev_t *indev = lv_indev_get_act();
  if (!indev)
    return;

  lv_point_t point;
  lv_indev_get_point(indev, &point);

  static lv_point_t last_point;
  static float start_gamma; // 用于记录开始滑动时的伽马值
  switch (code)
  {
  case LV_EVENT_PRESSED:
    last_point = point;
    start_gamma = current_gamma; // 记录开始值
    break;

  case LV_EVENT_PRESSING:
  {
    int32_t dy = point.y - last_point.y;
    if (abs(dy) > 1)
    { // 最小移动
      // 根据移动距离调整伽马值（向上滑动减小，向下滑动增加）
      current_gamma = start_gamma - (dy * SENSITIVITY);
      // 限制伽马值范围
      current_gamma = current_gamma < GAMMA_MIN ? GAMMA_MIN : (current_gamma > GAMMA_MAX ? GAMMA_MAX : current_gamma);
      // 更新系统伽马值
      set_gamma(current_gamma);
      last_point = point;
    }
    break;
  }
  case LV_EVENT_RELEASED:
    // 释放后
    break;
  }
}

void touch_gamma_init()
{
  lv_obj_add_event_cb(objects.panel_gamma_set, gamma_ctrl_cb, LV_EVENT_PRESSED, NULL);
  lv_obj_add_event_cb(objects.panel_gamma_set, gamma_ctrl_cb, LV_EVENT_PRESSING, NULL);
  lv_obj_add_event_cb(objects.panel_gamma_set, gamma_ctrl_cb, LV_EVENT_RELEASED, NULL);
}

#endif