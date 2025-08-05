#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 280, 240);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(obj, LV_DIR_NONE);
    lv_obj_set_scroll_snap_x(obj, LV_SCROLL_SNAP_NONE);
    lv_obj_set_scroll_snap_y(obj, LV_SCROLL_SNAP_NONE);
    {
        lv_obj_t *parent_obj = obj;
        {
            // t_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.t_1 = obj;
            lv_obj_set_pos(obj, 245, 32);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "999");
        }
        {
            // t_2
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.t_2 = obj;
            lv_obj_set_pos(obj, 245, 72);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "-100");
        }
        {
            // t_3
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.t_3 = obj;
            lv_obj_set_pos(obj, 247, 151);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "98");
        }
        {
            // t_4
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.t_4 = obj;
            lv_obj_set_pos(obj, 245, 176);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "Turb");
        }
        {
            // t_5
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.t_5 = obj;
            lv_obj_set_pos(obj, 245, 113);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "100°");
        }
        {
            // can_2
            lv_obj_t *obj = lv_canvas_create(parent_obj);
            objects.can_2 = obj;
            lv_obj_set_pos(obj, 241, 0);
            lv_obj_set_size(obj, 2, 240);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 245, 17);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "MAX");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 245, 57);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "MIN");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 245, 98);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "C_T");
        }
        {
            // label_bat
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_bat = obj;
            lv_obj_set_pos(obj, 249, 216);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "bat");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 245, 136);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "EMSI");
        }
        {
            // panel_gamma_set
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.panel_gamma_set = obj;
            lv_obj_set_pos(obj, 241, 0);
            lv_obj_set_size(obj, 40, 240);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // label_gamma
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_gamma = obj;
            lv_obj_set_pos(obj, 245, 192);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Gam");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_settings() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.settings = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 280, 240);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_scroll_dir(obj, LV_DIR_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 53, 4);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "发射率");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 29, 41);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "插值精度");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 0, 116);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "源滤波");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 5, 78);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "卡尔曼滤波");
        }
        {
            // epsilon
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.epsilon = obj;
            lv_obj_set_pos(obj, 129, 0);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_dropdown_set_options(obj, "100\n99\n98\n97\n96\n95\n94\n93\n92\n91\n90\n89\n88\n87\n86\n85\n84\n83\n82\n81\n80\n79\n78\n77\n76\n75\n74\n73\n72\n71\n70\n69\n68\n67\n66\n65\n64\n63\n62\n61\n60\n59\n58\n57\n56\n55\n54\n53\n52\n51\n50\n49\n48\n47\n46\n45\n44\n43\n42\n41\n40");
            lv_dropdown_set_selected(obj, 0);
            lv_obj_add_event_cb(obj, action_epsilon_dropdown, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            // bil_method
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.bil_method = obj;
            lv_obj_set_pos(obj, 130, 37);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_dropdown_set_options(obj, "0\n1\n2\n3");
            lv_dropdown_set_selected(obj, 0);
            lv_obj_add_event_cb(obj, action_bil_method_dropdown, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            // kalman_method
            lv_obj_t *obj = lv_dropdown_create(parent_obj);
            objects.kalman_method = obj;
            lv_obj_set_pos(obj, 130, 74);
            lv_obj_set_size(obj, 150, LV_SIZE_CONTENT);
            lv_dropdown_set_options(obj, "NONE\nSingle\nDouble");
            lv_dropdown_set_selected(obj, 0);
            lv_obj_add_event_cb(obj, action_kalman_method_dropdown, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            // sw_sfilter
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.sw_sfilter = obj;
            lv_obj_set_pos(obj, 75, 116);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, action_sw_sfilter_switch, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 129, 116);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "高温追踪");
        }
        {
            // sw_trace_max
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.sw_trace_max = obj;
            lv_obj_set_pos(obj, 229, 116);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, action_sw_trace_max_switch, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 11, 148);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "亮度");
        }
        {
            // but_save
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.but_save = obj;
            lv_obj_set_pos(obj, 215, 190);
            lv_obj_set_size(obj, 64, 50);
            lv_obj_add_event_cb(obj, action_but_save_button, LV_EVENT_CLICKED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "保存");
                }
            }
        }
        {
            // but_back
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.but_back = obj;
            lv_obj_set_pos(obj, 0, 190);
            lv_obj_set_size(obj, 65, 50);
            lv_obj_add_event_cb(obj, action_but_back_button, LV_EVENT_CLICKED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "返回");
                }
            }
        }
        {
            // brightness
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.brightness = obj;
            lv_obj_set_pos(obj, 65, 151);
            lv_obj_set_size(obj, 204, 23);
            lv_slider_set_range(obj, 10, 255);
            lv_slider_set_value(obj, 50, LV_ANIM_OFF);
            lv_obj_add_event_cb(obj, action_brightness_slider, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            // but_photo
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.but_photo = obj;
            lv_obj_set_pos(obj, 107, 190);
            lv_obj_set_size(obj, 67, 50);
            lv_obj_add_event_cb(obj, action_but_photo_button, LV_EVENT_CLICKED, (void *)0);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "照片");
                }
            }
        }
    }
    
    tick_screen_settings();
}

void tick_screen_settings() {
}

void create_screen_photo() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.photo = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 280, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // list_file
            lv_obj_t *obj = lv_list_create(parent_obj);
            objects.list_file = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 140, 240);
        }
        {
            // image_pic
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.image_pic = obj;
            lv_obj_set_pos(obj, 164, 10);
            lv_obj_set_size(obj, 96, 96);
        }
        {
            // but_del
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.but_del = obj;
            lv_obj_set_pos(obj, 163, 130);
            lv_obj_set_size(obj, 96, 50);
            lv_obj_add_event_cb(obj, action_but_del_button, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "删除");
                }
            }
        }
        {
            // but_back_2
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.but_back_2 = obj;
            lv_obj_set_pos(obj, 164, 186);
            lv_obj_set_size(obj, 96, 50);
            lv_obj_add_event_cb(obj, action_but_back_button, LV_EVENT_CLICKED, (void *)0);
            lv_obj_set_style_text_font(obj, &ui_font_font_c_24, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "返回");
                }
            }
        }
        {
            // lab_filename
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.lab_filename = obj;
            lv_obj_set_pos(obj, 171, 109);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "File_Name");
        }
    }
    
    tick_screen_photo();
}

void tick_screen_photo() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_settings,
    tick_screen_photo,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
    create_screen_settings();
    create_screen_photo();
}
