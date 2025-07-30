#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>
#define BAT_ADC  26

// 电池状态枚举
typedef enum {
    BATTERY_EMPTY,      // 空电量
    BATTERY_25,         // 25%电量
    BATTERY_50,         // 50%电量
    BATTERY_75,         // 75%电量
    BATTERY_FULL,       // 满电量
    BATTERY_CHARGING    // 充电中
} BatteryState;

// 电池状态与符号映射
static const char* battery_symbols[] = {
    [BATTERY_EMPTY]    = LV_SYMBOL_BATTERY_EMPTY,
    [BATTERY_25]       = LV_SYMBOL_BATTERY_1,
    [BATTERY_50]       = LV_SYMBOL_BATTERY_2,
    [BATTERY_75]       = LV_SYMBOL_BATTERY_3,
    [BATTERY_FULL]     = LV_SYMBOL_BATTERY_FULL,
    [BATTERY_CHARGING] = LV_SYMBOL_CHARGE
};

// 初始化电池监测系统
void battery_init() {
    pinMode(BAT_ADC, INPUT);
    int adc_value = analogRead(BAT_ADC);//似乎要读一次。要不然会出错
    lv_label_set_text(objects.label_bat, battery_symbols[BATTERY_50]);
}

// 根据电压获取电池状态
BatteryState get_battery_state(float voltage) {
    // 根据电压范围确定状态
    if (voltage < 3.4) return BATTERY_EMPTY;
    if (voltage < 3.5) return BATTERY_25;
    if (voltage < 3.7) return BATTERY_50;
    if (voltage < 4.0) return BATTERY_75;
    return BATTERY_FULL;
}

// 电压转百分比
uint8_t v_to_percent(float voltage) {
    // 简化的锂电池电压-电量模型
    if (voltage >= 4.2) return 100;
    if (voltage <= 3.0) return 0;
    
    // 线性插值 (3.0V-4.2V)
    return (uint8_t)((voltage - 3.0) * 100 / 1.2);
}

// 优化的电池监测循环
void battery_loop() {
    static unsigned long lastMillis = millis();  // 初始化为当前时间
    static const unsigned long xWait = 5000;   // 5秒更新间隔
    
    // 电阻分压系数 (预计算)
    static const float R1 = 300.0;
    static const float R2 = 680.0;
    static const float VOLTAGE_COEF = (R1 + R2) / R2 * (3.3 / 1024.0);

    // 检查是否需要更新
    unsigned long currentMillis = millis();
    if ((currentMillis - lastMillis) >= xWait) {
        // 读取ADC值 (多次采样平均)
        int adc_sum = 0;
        const int samples = 5;
        for (int i = 0; i < samples; i++) {
            adc_sum += analogRead(BAT_ADC);
            delay(2); // 短暂延时稳定
        }
        int adc_value = adc_sum / samples;
        // 计算电压
        float bat_v = adc_value * VOLTAGE_COEF;
        BatteryState state = get_battery_state(bat_v);
        // 更新电池图标
        lv_label_set_text(objects.label_bat, battery_symbols[state]);
        // 更新最后测量时间
        lastMillis = currentMillis;
    }
}

#endif