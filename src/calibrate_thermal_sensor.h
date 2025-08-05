#ifndef CALIBRATE_THERMAL_SENSOR_H
#define CALIBRATE_THERMAL_SENSOR_H


#include <FatFS.h>

 const char *config = "14,8,0,3000";


// 坏点矫正配置结构体
typedef struct {
    uint8_t row;        // 行坐标 (0-31)
    uint8_t col;        // 列坐标 (0-31)
    uint8_t mode;       // 0=固定偏移补偿, 1=邻域插值补偿
    int16_t offset;     // 仅mode=0时有效：补偿值（如-1000表示减去1000）
} CorrectionPoint;


// 矫正表存储（从文本加载）
static CorrectionPoint *correction_table = NULL;
static size_t correction_table_size = 0;


// 从文本字符串加载矫正表（格式："行,列,模式,偏移量;..."）
// 示例: "1,8,0,-1000;5,10,1,0;20,3,0,7500"
void load_correction_table(const char *text_config) {
    // 先释放旧表（如果存在）
    if (correction_table) {
        free(correction_table);
        correction_table = NULL;
        correction_table_size = 0;
    }
    
    // 计算配置项数量
    const char *ptr = text_config;
    size_t count = 1;  // 至少有一个配置项
    while (*ptr) {
        if (*ptr == ';') count++;
        ptr++;
    }
    
    // 分配内存
    correction_table = (CorrectionPoint *)malloc(count * sizeof(CorrectionPoint));
    if (!correction_table) return;
    
    // 解析文本配置
    char buffer[64];
    size_t index = 0;
    const char *start = text_config;
    
    while (index < count) {
        // 找到下一个分号或字符串结束
        const char *end = strchr(start, ';');
        if (!end) end = start + strlen(start);
        
        // 复制当前段到缓冲区
        size_t len = end - start;
        if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
        strncpy(buffer, start, len);
        buffer[len] = '\0';
        
        // 解析四个参数
        int row, col, mode, offset;
        if (sscanf(buffer, "%d,%d,%d,%d", &row, &col, &mode, &offset) == 4) {
            if (row >= 0 && row < 32 && col >= 0 && col < 32) {
                correction_table[index].row = (uint8_t)row;
                correction_table[index].col = (uint8_t)col;
                correction_table[index].mode = (uint8_t)mode;
                correction_table[index].offset = (int16_t)offset;
                index++;
            }
        }
        // 移动到下一段
        start = (*end == ';') ? end + 1 : end;
        if (*start == '\0') break;
    }
    
    correction_table_size = index;
}

// 释放矫正表内存
void free_correction_table() {
    if (correction_table) {
        free(correction_table);
        correction_table = NULL;
        correction_table_size = 0;
    }
}


// 矫正函数
void correct_thermal_data() {
 if (!correction_table || correction_table_size == 0) return;
    
    // 创建坏点标记矩阵
    uint32_t bad_pixel_map[32] = {0};
    for (size_t i = 0; i < correction_table_size; i++) {
        const CorrectionPoint* p = &correction_table[i];
        bad_pixel_map[p->row] |= (1U << p->col);
    }

    // 固定偏移补偿
    for (size_t i = 0; i < correction_table_size; i++) {
        const CorrectionPoint* p = &correction_table[i];
        if (p->mode != 0) continue;
        
        uint16_t* pixel = &new_data_pixel[p->row][p->col];

        int32_t new_value = (int32_t)(*pixel) + p->offset;
        if (new_value > 0xFFFF) {
            *pixel = 0xFFFF;
        } else if (new_value < 0) {
            *pixel = 0;
        } else {
            *pixel = (uint16_t)new_value;
        }

        bad_pixel_map[p->row] &= ~(1U << p->col);
    }
    // 邻域插值补偿
    for (size_t i = 0; i < correction_table_size; i++) {
        const CorrectionPoint* p = &correction_table[i];
        if (p->mode != 1) continue;  // 跳过非插值点
        
        uint16_t* target = &new_data_pixel[p->row][p->col];
        uint32_t sum = 0;
        uint8_t count = 0;
      
        // 检查8邻域 (行偏移: -1,0,1; 列偏移: -1,0,1)
        for (int dy = -1; dy <= 1; dy++) {
            int y = p->row + dy;
            if (y < 0 || y > 31) continue;  // 边界检查
            
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;  // 跳过自身
                
                int x = p->col + dx;
                if (x < 0 || x > 31) continue;  // 边界检查
                
                // 检查邻域点是否是好点（未被标记）
                if (!(bad_pixel_map[y] & (1U << x))) {
                    sum += new_data_pixel[y][x];
                    count++;
                }
            }
        }

        // 应用插值（至少需要4个有效邻域点）
        if (count >= 4) {
            *target = (uint16_t)(sum / count);  // 平均值插值
        } else if (count > 0) {
            *target = (uint16_t)(sum / count);  // 使用可用点
        } else {
            // 备用方案：使用最近邻（上方或左侧）
            *target = (p->row > 0) ? new_data_pixel[p->row-1][p->col] : 
                     (p->col > 0) ? new_data_pixel[p->row][p->col-1] : 0;
        }
        bad_pixel_map[p->row] &= ~(1U << p->col);  // 清除标记
    }
}






#endif