#ifndef SETTINGS_MANAGE_H
#define SETTINGS_MANAGE_H
#include <Arduino.h>
#include <EEPROM.h>
#include "shared_val.h"
#include "colormap.h"
#include "ui/ui.h"
#include "ui/screens.h"
#include <lvgl.h>
#include "ui/actions.h"
#include "Display.h"
#include "pic_manage.h"

#define FLAG_CONF_INITIALED 20

void update_settingsUI()
{
	// 更新设置界面
	lv_dropdown_set_selected(objects.bil_method, interpolation_method); // 双线性
	lv_dropdown_set_selected(objects.kalman_method, use_kalman_model);	// 卡尔曼
	lv_dropdown_set_selected(objects.epsilon, 100 - epsilon);			// 发射率
	// lv_dropdown_set_text(objects.epsilon, epsilon);
	lv_slider_set_value(objects.brightness, brightness, LV_ANIM_ON);

	if (use_sfilter)
		lv_obj_add_state(objects.sw_sfilter, LV_STATE_CHECKED);
	if (flag_trace_max)
		lv_obj_add_state(objects.sw_trace_max, LV_STATE_CHECKED);
}

// 将配置保存到不掉电存储器中
void eeprom_save_config()
{
	EEPROM.write(0, FLAG_CONF_INITIALED);
	EEPROM.write(1, (uint8_t)brightness);			// 亮度
	EEPROM.write(2, (uint8_t)use_kalman_model);		// 卡尔曼模型
	EEPROM.write(3, (uint8_t)interpolation_method); // 双线性滤波精度
	EEPROM.write(4, use_sfilter);					// 平滑滤波
	EEPROM.write(5, current_preset);
	EEPROM.write(6, flag_show_temp_text); // 成像图显示温度
	EEPROM.write(7, flag_trace_max);	  // 是否追踪热点
	EEPROM.commit();
}

void load_settings()
{
	EEPROM.begin(128);
	delay(5);
	if (EEPROM.read(0) != FLAG_CONF_INITIALED)
	{
		eeprom_save_config();
	}
	else
	{
		brightness = EEPROM.read(1);
		use_kalman_model = EEPROM.read(2);
		interpolation_method = EEPROM.read(3);
		use_sfilter = EEPROM.read(4) != 0;
		current_preset = (ColormapPreset)EEPROM.read(5);
		if (current_preset > COLORMAP_PRESET_COUNT)
		{
			current_preset = RAINBOW2;
		}
		flag_show_temp_text = EEPROM.read(6) != 0;
		flag_trace_max = EEPROM.read(7) != 0;
	}
	load_colormap(current_preset);
	update_settingsUI();
}

void action_sw_sfilter_switch(lv_event_t *e)
{
	use_sfilter = lv_obj_has_state(objects.sw_sfilter, LV_STATE_CHECKED);
}

void action_sw_trace_max_switch(lv_event_t *e)
{
	flag_trace_max = lv_obj_has_state(objects.sw_trace_max, LV_STATE_CHECKED);
}
void action_brightness_slider(lv_event_t *e)
{
	brightness = lv_slider_get_value(objects.brightness);
	set_brightness(brightness);
}

void action_kalman_method_dropdown(lv_event_t *e)
{
	use_kalman_model = lv_dropdown_get_selected(objects.kalman_method);
}

void action_bil_method_dropdown(lv_event_t *e)
{
	interpolation_method = lv_dropdown_get_selected(objects.bil_method);
}

void action_epsilon_dropdown(lv_event_t *e)
{
	setfsl(100 - lv_dropdown_get_selected(objects.epsilon));
}

void action_but_save_button(lv_event_t *e)
{
	eeprom_save_config();
}

void action_but_back_button(lv_event_t *e)
{
	toMainScreen();
}

void action_but_photo_button(lv_event_t *e)
{
	loadScreen(SCREEN_ID_PHOTO);
	load_datalist();
}

#endif