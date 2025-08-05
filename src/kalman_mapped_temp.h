// kalman_mapped_temp_ushort.h
#ifndef KALMAN_MAPPED_TEMP_H
#define KALMAN_MAPPED_TEMP_H

#include <Arduino.h>

#define GRID_SIZE 32
#define COLOR_LEVELS color_num

typedef struct {
    float P;        // 估算协方差
    float G;        // 卡尔曼增益
    float Output;   // 滤波器输出 (映射值 0-179)
} KFPType2;

class KalmanFilterMappedUShort {
private:
    KFPType2 filters2[GRID_SIZE][GRID_SIZE];  // 32x32滤波器数组
    float Q;        // 过程噪声协方差
    float R;        // 测量噪声协方差
    bool initialized;

public:
    // 构造函数
    KalmanFilterMappedUShort(float q = 0.05, float r = 0.1) 
        : Q(q), R(r), initialized(false) {}
    
    // 初始化滤波器
    void init() {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                filters2[i][j] = {0.2, 0, 90}; // 初始值设为中间值90
            }
        }
        initialized = true;
    }
    
    // 设置噪声参数
    void setNoiseParams(float q, float r) {
        Q = q;
        R = r;
    }
    
    // 直接处理原数组 (unsigned short类型)
    void processFrame(unsigned short data[GRID_SIZE][GRID_SIZE]) 
    {
        if (!initialized) init();
        
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                KFPType2* kfp = &filters2[i][j];
                
                // 保存当前值（滤波前）并转换为浮点
                float current_value = static_cast<float>(data[i][j]);
                
                // 预测步骤
                kfp->P = kfp->P + Q;
                
                // 更新步骤
                kfp->G = kfp->P / (kfp->P + R);
                kfp->Output = kfp->Output + kfp->G * (current_value - kfp->Output);
                kfp->P = (1 - kfp->G) * kfp->P;
                
                // 约束在0-179范围并直接更新原数组
                float filtered = kfp->Output;
                if (filtered < 0) filtered = 0;
                if (filtered > COLOR_LEVELS-1) filtered = COLOR_LEVELS-1;
                
                data[i][j] = static_cast<unsigned short>(filtered + 0.5f); // 四舍五入
                kfp->Output = filtered; // 更新状态
            }
        }
    }
    
    // 获取滤波后映射值
    unsigned short getFilteredValue(int i, int j) {
        return static_cast<unsigned short>(filters2[i][j].Output + 0.5f);
    }
};

#endif // KALMAN_MAPPED_TEMP_H