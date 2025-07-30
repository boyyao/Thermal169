#ifndef KALMAN_RAW_TEMP_H
#define KALMAN_RAW_TEMP_H

#include <Arduino.h>

#define GRID_SIZE 32

typedef struct {
    float P;        // 估算协方差
    float G;        // 卡尔曼增益
    float Output;   // 滤波器输出 (开尔文温度)
} KFPType;

class KalmanFilterRawUShort {
private:
    KFPType filters[GRID_SIZE][GRID_SIZE];  // 32x32滤波器数组
    float Q;        // 过程噪声协方差
    float R;        // 测量噪声协方差
    unsigned short min_temp; // 场景最低温度(乘以10)
    unsigned short max_temp; // 场景最高温度(乘以10)
    bool initialized;

public:
    // 构造函数
    KalmanFilterRawUShort(float q = 1.0, float r = 1.5) 
        : Q(q), R(r), min_temp(2930), max_temp(3130), initialized(false) {}
    
    // 初始化滤波器
    void init(unsigned short initial_temp = 2980) {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                filters[i][j] = {0.2, 0, initial_temp / 10.0f}; // 转换为实际温度值
            }
        }
        initialized = true;
    }
    
    // 更新温度范围 (输入乘以10的整数值)
    void updateTempRange(unsigned short min, unsigned short max) {
        min_temp = min;
        max_temp = max;
    }
    
    // 设置噪声参数
    void setNoiseParams(float q, float r) {
        Q = q;
        R = r;
    }
    
    // 直接处理原数组
    void processFrame(unsigned short data[GRID_SIZE][GRID_SIZE]) 
    {
        if (!initialized) init();
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                KFPType* kfp = &filters[i][j];
                // 保存当前值（滤波前）
                unsigned short current_value = data[i][j];
                // 转换为实际温度值
                float actual_input = current_value / 10.0f;
                // 预测步骤
                kfp->P = kfp->P + Q;
                // 更新步骤
                kfp->G = kfp->P / (kfp->P + R);
                kfp->Output = kfp->Output + kfp->G * (actual_input - kfp->Output);
                kfp->P = (1 - kfp->G) * kfp->P;
                // 直接更新原数组：转换回乘以10的整数值
                data[i][j] = static_cast<unsigned short>(kfp->Output * 10 + 0.5f);
            }
        }
    }
    
    // 获取滤波后温度 (实际温度值)
    float getFilteredTemp(int i, int j) {
        return filters[i][j].Output;
    }
    
    // 获取滤波后原始值 (乘以10的整数值)
    unsigned short getFilteredValue(int i, int j) {
        return static_cast<unsigned short>(filters[i][j].Output * 10 + 0.5f);
    }
};

#endif // KALMAN_RAW_TEMP_USHORT_H