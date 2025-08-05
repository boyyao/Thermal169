#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_sw_sfilter_switch(lv_event_t * e);
extern void action_sw_trace_max_switch(lv_event_t * e);
extern void action_brightness_slider(lv_event_t * e);
extern void action_kalman_method_dropdown(lv_event_t * e);
extern void action_bil_method_dropdown(lv_event_t * e);
extern void action_epsilon_dropdown(lv_event_t * e);
extern void action_but_save_button(lv_event_t * e);
extern void action_but_back_button(lv_event_t * e);
extern void action_but_photo_button(lv_event_t * e);
extern void action_but_del_button(lv_event_t * e);
extern void action_set_gamma(lv_event_t * e);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/