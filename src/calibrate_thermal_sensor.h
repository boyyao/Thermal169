#ifndef CALIBRATE_THERMAL_SENSOR_H
#define CALIBRATE_THERMAL_SENSOR_H

#include <FatFS.h>

//直接配置
//const char *config = "14,8,0,3000";

// 坏点矫正配置结构体
typedef struct {
    uint8_t row;        // 行坐标 (0-based)
    uint8_t col;        // 列坐标 (0-based)
    uint8_t mode;       // 0=固定偏移补偿, 1=邻域插值补偿
    int16_t offset;     // 补偿值
} CorrectionPoint;

// 矫正表存储
static CorrectionPoint *correction_table = NULL;
static size_t correction_table_size = 0;
static const size_t MAX_CALIB_POINTS = 1024; // 最大校准点数


// 从文本字符串加载矫正表（格式："行,列,模式,偏移量;..."）
// 示例: "1,8,0,-1000;5,10,1,0;20,3,0,7500"
void load_correction_table(const char *text_config) {
    // 释放旧表
    if (correction_table) {
        free(correction_table);
        correction_table = NULL;
        correction_table_size = 0;
    }
    
    // 计算配置项数量
    const char *ptr = text_config;
    size_t count = 1;  // 至少有一个配置项
    while (*ptr) {
        // 支持多种分隔符：分号、换行(\n)、回车换行(\r\n)
        if (*ptr == ';' || *ptr == '\n') count++;
        ptr++;
    }

    // 限制最大点数
    if (count > MAX_CALIB_POINTS) count = MAX_CALIB_POINTS;
    
    // 分配内存
    correction_table = (CorrectionPoint *)malloc(count * sizeof(CorrectionPoint));
    if (!correction_table) {
        printf("内存分配失败，请求大小: %d\n", count * sizeof(CorrectionPoint));
        return;
    }
    
    // 解析文本配置
    char buffer[64];
    size_t index = 0;
    const char *start = text_config;
    
    while (index < count && *start) {
        // 找到下一个分隔符（分号、换行或回车）
        const char *end = start;
        while (*end && *end != ';' && *end != '\n' && *end != '\r') end++;
        
        // 复制当前段到缓冲区
        size_t len = end - start;
        if (len > 0) {
            if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
            strncpy(buffer, start, len);
            buffer[len] = '\0';
            
            // 移除可能的回车符（处理 \r\n 情况）
            if (len > 0 && buffer[len-1] == '\r') {
                buffer[len-1] = '\0';
                len--;
            }
            
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
        }
        // 移动到下一段，跳过所有分隔符
        start = end;
        while (*start == ';' || *start == '\n' || *start == '\r') start++;
    }
    correction_table_size = index;
    printf("加载 %d 条校准配置\n", index);
}


// 释放矫正表内存
void free_correction_table() {
    if (correction_table) {
        free(correction_table);
        correction_table = NULL;
        correction_table_size = 0;
    }
}

// 创建默认配置文件
void create_default_calib_file(const char *filename) {
    File file = FatFS.open(filename, "w");
    if (!file) {
        printf("创建校准文件失败\n");
        return;
    }
    // 写入文件头说明（使用 \r\n 兼容 Windows）
    file.println("# 热成像坏点矫正配置文件\r");
    file.println("# 格式: 行,列,模式,偏移量\r");
    file.println("# 行/列: 0-based索引 (0-31)\r");
    file.println("# 模式: 0=固定偏移补偿, 1=邻域插值补偿\r");
    file.println("# 偏移量: 固定模式下的补偿值（正数=增加，负数=减少）\r");
    file.println("# 示例:\r");
    file.println("# 14,8,0,3000   # 第15行第9列增加3000\r");
    file.println("# 8,14,1,0      # 第9行第15列使用邻域插值\r");
    file.close();
    printf("已创建默认校准文件: %s\n", filename);
}



// 从文件加载校准配置
void load_correction_from_file(const char *filename) {
    // 尝试打开文件
    File file = FatFS.open(filename, "r");
    if (!file) {
        printf("未找到校准文件 %s，创建默认文件\n", filename);
        create_default_calib_file(filename);
        file = FatFS.open(filename, "r"); // 重新打开
        if (!file) {
            printf("无法打开默认文件\n");
            return;
        }
    }
    
    // 获取文件大小
    size_t file_size = file.size();
    if (file_size == 0) {
        printf("空文件\n");
        file.close();
        return;
    }
    
    // 分配缓冲区（+1用于终止符）
    char *buffer = (char *)malloc(file_size + 1);
    if (!buffer) {
        printf("内存分配失败\n");
        file.close();
        return;
    }
    // 读取整个文件
    size_t bytes_read = file.read((uint8_t *)buffer, file_size);
    buffer[bytes_read] = '\0';  // 添加终止符
    file.close();
    // 调用核心加载函数
    load_correction_table(buffer);
    // 释放缓冲区
    free(buffer);
}

// 保存校准配置文件
void save_correction_to_file(const char *filename) {
    File file = FatFS.open(filename, "w");
    if (!file) {
        printf("保存校准文件失败: %s\n", filename);
        return;
    }
    
    // 写入文件头（使用 \r\n 兼容 Windows）
    file.println("# 热成像坏点矫正配置文件\r");
    file.println("# 格式: 行,列,模式,偏移量\r");
    file.println("# 行/列: 0-based索引 (0-31)\r");
    
    // 写入所有配置项（使用 \r\n 换行）
    for (size_t i = 0; i < correction_table_size; i++) {
        const CorrectionPoint* p = &correction_table[i];
        file.printf("%d,%d,%d,%d\r\n", 
                   p->row, p->col, p->mode, p->offset);
    }
    
    file.close();
    printf("已保存 %d 条校准配置到 %s\n", correction_table_size, filename);
}

// ==================== 辅助函数 ====================
// 移除字符串中的回车符（处理 \r\n 情况）
void remove_carriage_returns(char *str) {
    char *src = str;
    char *dst = str;
    
    while (*src) {
        if (*src != '\r') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
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