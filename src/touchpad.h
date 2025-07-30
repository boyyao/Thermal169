#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include "CST816T.h"

#define TOUCH_SDA 2
#define TOUCH_SCL 3
#define TOUCH_RST -1
#define ROTATE 2

CST816T touch(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, -1); // sda, scl, rst, irq

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    touch.update();
    static uint16_t x, y;       // LVGL坐标系
    static uint16_t tftx, tfty; // TFT eSPI坐标系
    if (!touch.tp.touching)
    // if( 0!=touch.data.points )
    {
        // 触摸释放
        data->state = LV_INDEV_STATE_RELEASED;
    }
    else
    {
        // 触摸按下
        data->state = LV_INDEV_STATE_PRESSED;
#if (ROTATE == 0)
        x = touch.tp.x;
        y = touch.tp.y;
#endif
#if (ROTATE == 1)
        x = touch.tp.y;
        y = 240 - touch.tp.x;
#endif
#if (ROTATE == 2)
        x = 240 - touch.tp.x;
        y = 280 - touch.tp.y;
#endif

#if (ROTATE == 3)
        x = 280 - touch.tp.y;
        y = touch.tp.x;
#endif
        if (in_settings)
        {
            data->point.x = x;
            data->point.y = y;
            return;
        }

        tftx = touch.tp.y;
        tfty = 240 - touch.tp.x;
        if (tftx < 230)
        { // 点击屏幕光标
            flag_show_cursor = true;
            test_point[0] = tftx;
            test_point[1] = tfty;
        }
    }
}

#endif