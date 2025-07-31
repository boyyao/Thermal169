#ifndef KALMAN_RAW_TEMP_H
#define KALMAN_RAW_TEMP_H

#include <Arduino.h>

#define GRID_SIZE 32
#define NUM_PIXELS (GRID_SIZE * GRID_SIZE)

typedef int32_t fixed_point;
#define FIXED_SHIFT 8
#define FLOAT_TO_FIXED(x) ((fixed_point)((x) * (1 << FIXED_SHIFT)))
#define FIXED_TO_FLOAT(x) ((float)(x) / (1 << FIXED_SHIFT))
#define INT_TO_FIXED(x) ((fixed_point)(x) << FIXED_SHIFT)
#define FIXED_TO_INT(x) ((x) >> FIXED_SHIFT)
#define FIXED_MULT(a, b) ((fixed_point)(((int64_t)(a) * (b)) >> FIXED_SHIFT))

// 卡尔曼滤波器结构体
typedef struct {
    fixed_point state;      // 当前状态估计
    fixed_point covariance; // 状态协方差
} KalmanFilter;

// 全局滤波器数组
KalmanFilter filters[GRID_SIZE][GRID_SIZE];

const fixed_point PROCESS_NOISE = FLOAT_TO_FIXED(0.1f);  // 增加过程噪声
const fixed_point SENSOR_NOISE = FLOAT_TO_FIXED(1.0f);   // 减小传感器噪声权重

void initKalmanFilters(uint16_t initialValue) {
    fixed_point initState = INT_TO_FIXED(initialValue);
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            filters[y][x].state = initState;
            filters[y][x].covariance = SENSOR_NOISE; 
        }
    }
}

// 处理单帧热成像数据
void processFrame(uint16_t tempData[GRID_SIZE][GRID_SIZE]) {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            KalmanFilter* kf = &filters[y][x];
            uint16_t rawValue = tempData[y][x];
            
            // 1. 预测阶段
            fixed_point pred_cov = kf->covariance + PROCESS_NOISE;
            
            // 2. 计算卡尔曼增益
            fixed_point denominator = pred_cov + SENSOR_NOISE;
            
            // 使用64位整数避免溢出
            int64_t gain_numerator = (int64_t)pred_cov << FIXED_SHIFT;
            int64_t gain = (gain_numerator / denominator);
            
            // 3. 状态更新
            fixed_point innovation = INT_TO_FIXED(rawValue) - kf->state;
            kf->state += FIXED_MULT((fixed_point)gain, innovation);
            
            // 4. 协方差更新
            fixed_point one_minus_gain = (1 << FIXED_SHIFT) - gain;
            kf->covariance = FIXED_MULT(one_minus_gain, pred_cov);
            
            // 5. 将滤波后的值写回 (四舍五入)
            fixed_point result = kf->state + (1 << (FIXED_SHIFT - 1));
            tempData[y][x] = FIXED_TO_INT(result);
        }
    }
}

#endif