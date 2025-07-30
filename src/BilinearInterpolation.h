#ifndef BIO_INTERPOLATION_MULTI_H
#define BIO_INTERPOLATION_MULTI_H

#include <Arduino.h>

// 定义目标分辨率选项
enum TargetResolution {
    RES_240x240 = 0,  // 240x240 分辨率
    RES_96x96 = 1     // 96x96 分辨率
};

// 定义插值方法枚举
enum InterpolationMethod {
    NEAREST_NEIGHBOR = 0,  // 无小数
    BILINEAR_1BIT,         // 1位小数
    BILINEAR_2BIT,         // 2位小数
    BILINEAR_4BIT          // 4位小数
};

// ================== 分辨率独立的状态标志 ==================
struct ResolutionState {
    bool common_initialized = false;
    bool frac_initialized = false;
    bool nn_initialized = false;
};

// 为每种分辨率创建独立的状态
static ResolutionState state_240;
static ResolutionState state_96;

// ================== 共享表结构 ==================
// 所有双线性插值方法共享的整数坐标表
static uint8_t common_x0_240[240];
static uint8_t common_x1_240[240];
static uint8_t common_y0_240[240];
static uint8_t common_y1_240[240];

static uint8_t common_x0_96[96];
static uint8_t common_x1_96[96];
static uint8_t common_y0_96[96];
static uint8_t common_y1_96[96];

// 各精度专用的分数表
static uint8_t frac_1bit_fx_240[240];
static uint8_t frac_1bit_fy_240[240];
static uint8_t frac_2bit_fx_240[240];
static uint8_t frac_2bit_fy_240[240];
static uint8_t frac_4bit_fx_240[240];
static uint8_t frac_4bit_fy_240[240];

static uint8_t frac_1bit_fx_96[96];
static uint8_t frac_1bit_fy_96[96];
static uint8_t frac_2bit_fx_96[96];
static uint8_t frac_2bit_fy_96[96];
static uint8_t frac_4bit_fx_96[96];
static uint8_t frac_4bit_fy_96[96];

// ================== 最近邻表 ==================
static uint8_t nn_x_map_240[240];
static uint8_t nn_y_map_240[240];
static uint8_t nn_x_map_96[96];
static uint8_t nn_y_map_96[96];

// ================== 共享初始化函数 ==================
void init_common_tables(TargetResolution resolution) {
    ResolutionState& state = (resolution == RES_240x240) ? state_240 : state_96;
    if (state.common_initialized) return;
    
    // 根据分辨率选择参数
    int DST_W, DST_H;
    float scale;
    uint8_t* x0_table;
    uint8_t* x1_table;
    uint8_t* y0_table;
    uint8_t* y1_table;
    
    if (resolution == RES_240x240) {
        DST_W = 240;
        DST_H = 240;
        scale = 32.0f / 240.0f;
        x0_table = common_x0_240;
        x1_table = common_x1_240;
        y0_table = common_y0_240;
        y1_table = common_y1_240;
    } else { // RES_96x96
        DST_W = 96;
        DST_H = 96;
        scale = 32.0f / 96.0f;
        x0_table = common_x0_96;
        x1_table = common_x1_96;
        y0_table = common_y0_96;
        y1_table = common_y1_96;
    }
    
    // 计算所有方法共享的整数坐标
    for (int dx = 0; dx < DST_W; dx++) {
        float src_x = (dx + 0.5f) * scale - 0.5f;
        
        if (src_x < 0) {
            x0_table[dx] = 0;
            x1_table[dx] = 0;
        } else if (src_x >= 31) {
            x0_table[dx] = 31;
            x1_table[dx] = 31;
        } else {
            int x0 = static_cast<int>(src_x);
            x0_table[dx] = x0;
            x1_table[dx] = x0 + 1;
        }
    }
    
    for (int dy = 0; dy < DST_H; dy++) {
        float src_y = (dy + 0.5f) * scale - 0.5f;
        
        if (src_y < 0) {
            y0_table[dy] = 0;
            y1_table[dy] = 0;
        } else if (src_y >= 31) {
            y0_table[dy] = 31;
            y1_table[dy] = 31;
        } else {
            int y0 = static_cast<int>(src_y);
            y0_table[dy] = y0;
            y1_table[dy] = y0 + 1;
        }
    }
    
    state.common_initialized = true;
}

// ================== 函数声明 ==================
uint16_t bio_nearest_neighbor(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]);
uint16_t bio_bilinear_1bit(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]);
uint16_t bio_bilinear_2bit(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]);
uint16_t bio_bilinear_4bit(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]);

// ================== 函数指针类型 ==================
typedef uint16_t (*InterpolationFunc)(TargetResolution, int, int, uint16_t[32][32]);

// ================== 函数指针数组 ==================
static const InterpolationFunc interpolation_funcs[] = {
    bio_nearest_neighbor,   // 索引 0
    bio_bilinear_1bit,      // 索引 1
    bio_bilinear_2bit,      // 索引 2
    bio_bilinear_4bit       // 索引 3
};

// ================== 统一调用接口 ==================
inline uint16_t bio_interpolate(TargetResolution resolution, int method, 
                               int dst_x, int dst_y, uint16_t src_data[32][32]) {
    return interpolation_funcs[method](resolution, dst_x, dst_y, src_data);
}

// ================== 分数表初始化 ==================
void init_frac_tables(TargetResolution resolution) {
    ResolutionState& state = (resolution == RES_240x240) ? state_240 : state_96;
    if (state.frac_initialized) return;
    
    // 确保共享坐标表已初始化
    if (!state.common_initialized) init_common_tables(resolution);
    
    // 根据分辨率选择参数
    int DST_W, DST_H;
    float scale;
    uint8_t* frac1_fx;
    uint8_t* frac1_fy;
    uint8_t* frac2_fx;
    uint8_t* frac2_fy;
    uint8_t* frac4_fx;
    uint8_t* frac4_fy;
    uint8_t* x0_table;
    uint8_t* y0_table;
    
    if (resolution == RES_240x240) {
        DST_W = 240;
        DST_H = 240;
        scale = 32.0f / 240.0f;
        frac1_fx = frac_1bit_fx_240;
        frac1_fy = frac_1bit_fy_240;
        frac2_fx = frac_2bit_fx_240;
        frac2_fy = frac_2bit_fy_240;
        frac4_fx = frac_4bit_fx_240;
        frac4_fy = frac_4bit_fy_240;
        x0_table = common_x0_240;
        y0_table = common_y0_240;
    } else { // RES_96x96
        DST_W = 96;
        DST_H = 96;
        scale = 32.0f / 96.0f;
        frac1_fx = frac_1bit_fx_96;
        frac1_fy = frac_1bit_fy_96;
        frac2_fx = frac_2bit_fx_96;
        frac2_fy = frac_2bit_fy_96;
        frac4_fx = frac_4bit_fx_96;
        frac4_fy = frac_4bit_fy_96;
        x0_table = common_x0_96;
        y0_table = common_y0_96;
    }
    
    // 计算1位精度分数表
    for (int dx = 0; dx < DST_W; dx++) {
        float src_x = (dx + 0.5f) * scale - 0.5f;
        
        if (src_x < 0 || src_x >= 31) {
            frac1_fx[dx] = 0;
        } else {
            int x0 = x0_table[dx];
            frac1_fx[dx] = (src_x - x0 >= 0.5f) ? 1 : 0;
        }
    }
    
    for (int dy = 0; dy < DST_H; dy++) {
        float src_y = (dy + 0.5f) * scale - 0.5f;
        
        if (src_y < 0 || src_y >= 31) {
            frac1_fy[dy] = 0;
        } else {
            int y0 = y0_table[dy];
            frac1_fy[dy] = (src_y - y0 >= 0.5f) ? 1 : 0;
        }
    }
    
    // 计算2位精度分数表
    for (int dx = 0; dx < DST_W; dx++) {
        float src_x = (dx + 0.5f) * scale - 0.5f;
        
        if (src_x < 0 || src_x >= 31) {
            frac2_fx[dx] = 0;
        } else {
            int x0 = x0_table[dx];
            float frac = src_x - x0;
            frac2_fx[dx] = static_cast<uint8_t>(frac * 4);
        }
    }
    
    for (int dy = 0; dy < DST_H; dy++) {
        float src_y = (dy + 0.5f) * scale - 0.5f;
        
        if (src_y < 0 || src_y >= 31) {
            frac2_fy[dy] = 0;
        } else {
            int y0 = y0_table[dy];
            float frac = src_y - y0;
            frac2_fy[dy] = static_cast<uint8_t>(frac * 4);
        }
    }
    
    // 计算4位精度分数表
    for (int dx = 0; dx < DST_W; dx++) {
        float src_x = (dx + 0.5f) * scale - 0.5f;
        
        if (src_x < 0 || src_x >= 31) {
            frac4_fx[dx] = 0;
        } else {
            int x0 = x0_table[dx];
            float frac = src_x - x0;
            frac4_fx[dx] = static_cast<uint8_t>(frac * 16);
        }
    }
    
    for (int dy = 0; dy < DST_H; dy++) {
        float src_y = (dy + 0.5f) * scale - 0.5f;
        
        if (src_y < 0 || src_y >= 31) {
            frac4_fy[dy] = 0;
        } else {
            int y0 = y0_table[dy];
            float frac = src_y - y0;
            frac4_fy[dy] = static_cast<uint8_t>(frac * 16);
        }
    }
    
    state.frac_initialized = true;
}

// ================== 最近邻插值实现 ==================
void init_nearest_neighbor_tables(TargetResolution resolution) {
    ResolutionState& state = (resolution == RES_240x240) ? state_240 : state_96;
    if (state.nn_initialized) return;
    
    // 根据分辨率选择参数
    int DST_W, DST_H;
    float scale;
    uint8_t* x_map;
    uint8_t* y_map;
    
    if (resolution == RES_240x240) {
        DST_W = 240;
        DST_H = 240;
        scale = 32.0f / 240.0f;
        x_map = nn_x_map_240;
        y_map = nn_y_map_240;
    } else { // RES_96x96
        DST_W = 96;
        DST_H = 96;
        scale = 32.0f / 96.0f;
        x_map = nn_x_map_96;
        y_map = nn_y_map_96;
    }
    
    for (int dx = 0; dx < DST_W; dx++) {
        float src_x = dx * scale;
        x_map[dx] = static_cast<uint8_t>(src_x + 0.5f);
        if (x_map[dx] >= 32) x_map[dx] = 31;
    }
    
    for (int dy = 0; dy < DST_H; dy++) {
        float src_y = dy * scale;
        y_map[dy] = static_cast<uint8_t>(src_y + 0.5f);
        if (y_map[dy] >= 32) y_map[dy] = 31;
    }
    
    state.nn_initialized = true;
}

uint16_t bio_nearest_neighbor(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]) {
    ResolutionState& state = (resolution == RES_240x240) ? state_240 : state_96;
    if (!state.nn_initialized) init_nearest_neighbor_tables(resolution);
    
    if (resolution == RES_240x240) {
        return src_data[nn_y_map_240[dst_y]][nn_x_map_240[dst_x]];
    } else { // RES_96x96
        return src_data[nn_y_map_96[dst_y]][nn_x_map_96[dst_x]];
    }
}

// ================== 双线性插值实现（共享表） ==================
uint16_t bio_bilinear_1bit(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]) {
    ResolutionState& state = (resolution == RES_240x240) ? state_240 : state_96;
    if (!state.frac_initialized) init_frac_tables(resolution);
    
    uint8_t x0, x1, fx, y0, y1, fy;
    
    if (resolution == RES_240x240) {
        x0 = common_x0_240[dst_x];
        x1 = common_x1_240[dst_x];
        fx = frac_1bit_fx_240[dst_x];
        y0 = common_y0_240[dst_y];
        y1 = common_y1_240[dst_y];
        fy = frac_1bit_fy_240[dst_y];
    } else { // RES_96x96
        x0 = common_x0_96[dst_x];
        x1 = common_x1_96[dst_x];
        fx = frac_1bit_fx_96[dst_x];
        y0 = common_y0_96[dst_y];
        y1 = common_y1_96[dst_y];
        fy = frac_1bit_fy_96[dst_y];
    }
    
    uint16_t v00 = src_data[y0][x0];
    uint16_t v01 = src_data[y0][x1];
    uint16_t v10 = src_data[y1][x0];
    uint16_t v11 = src_data[y1][x1];
    
    uint16_t top = (fx == 0) ? v00 : ((v00 + v01) >> 1);
    uint16_t bottom = (fx == 0) ? v10 : ((v10 + v11) >> 1);
    
    return (fy == 0) ? top : ((top + bottom) >> 1);
}

uint16_t bio_bilinear_2bit(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]) {
    ResolutionState& state = (resolution == RES_240x240) ? state_240 : state_96;
    if (!state.frac_initialized) init_frac_tables(resolution);
    
    uint8_t x0, x1, fx, y0, y1, fy;
    
    if (resolution == RES_240x240) {
        x0 = common_x0_240[dst_x];
        x1 = common_x1_240[dst_x];
        fx = frac_2bit_fx_240[dst_x];
        y0 = common_y0_240[dst_y];
        y1 = common_y1_240[dst_y];
        fy = frac_2bit_fy_240[dst_y];
    } else { // RES_96x96
        x0 = common_x0_96[dst_x];
        x1 = common_x1_96[dst_x];
        fx = frac_2bit_fx_96[dst_x];
        y0 = common_y0_96[dst_y];
        y1 = common_y1_96[dst_y];
        fy = frac_2bit_fy_96[dst_y];
    }
    
    uint16_t v00 = src_data[y0][x0];
    uint16_t v01 = src_data[y0][x1];
    uint16_t v10 = src_data[y1][x0];
    uint16_t v11 = src_data[y1][x1];
    
    uint16_t top = v00 * (4 - fx) + v01 * fx;
    uint16_t bottom = v10 * (4 - fx) + v11 * fx;
    
    return (top * (4 - fy) + bottom * fy) >> 4;
}

uint16_t bio_bilinear_4bit(TargetResolution resolution, int dst_x, int dst_y, uint16_t src_data[32][32]) {
    ResolutionState& state = (resolution == RES_240x240) ? state_240 : state_96;
    if (!state.frac_initialized) init_frac_tables(resolution);
    
    uint8_t x0, x1, fx, y0, y1, fy;
    
    if (resolution == RES_240x240) {
        x0 = common_x0_240[dst_x];
        x1 = common_x1_240[dst_x];
        fx = frac_4bit_fx_240[dst_x];
        y0 = common_y0_240[dst_y];
        y1 = common_y1_240[dst_y];
        fy = frac_4bit_fy_240[dst_y];
    } else { // RES_96x96
        x0 = common_x0_96[dst_x];
        x1 = common_x1_96[dst_x];
        fx = frac_4bit_fx_96[dst_x];
        y0 = common_y0_96[dst_y];
        y1 = common_y1_96[dst_y];
        fy = frac_4bit_fy_96[dst_y];
    }
    
    uint16_t v00 = src_data[y0][x0];
    uint16_t v01 = src_data[y0][x1];
    uint16_t v10 = src_data[y1][x0];
    uint16_t v11 = src_data[y1][x1];
    
    uint16_t top = v00 * (16 - fx) + v01 * fx;
    uint16_t bottom = v10 * (16 - fx) + v11 * fx;
    
    return (top * (16 - fy) + bottom * fy) >> 8;
}

#endif