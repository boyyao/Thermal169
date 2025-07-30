#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *settings;
    lv_obj_t *photo;
    lv_obj_t *t_1;
    lv_obj_t *t_2;
    lv_obj_t *t_3;
    lv_obj_t *t_4;
    lv_obj_t *t_5;
    lv_obj_t *can_2;
    lv_obj_t *label_bat;
    lv_obj_t *epsilon;
    lv_obj_t *bil_method;
    lv_obj_t *kalman_method;
    lv_obj_t *sw_sfilter;
    lv_obj_t *sw_trace_max;
    lv_obj_t *but_save;
    lv_obj_t *but_back;
    lv_obj_t *brightness;
    lv_obj_t *but_photo;
    lv_obj_t *list_file;
    lv_obj_t *image_pic;
    lv_obj_t *but_del;
    lv_obj_t *but_back_2;
    lv_obj_t *lab_filename;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS = 2,
    SCREEN_ID_PHOTO = 3,
};

void create_screen_main();
void tick_screen_main();

void create_screen_settings();
void tick_screen_settings();

void create_screen_photo();
void tick_screen_photo();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/