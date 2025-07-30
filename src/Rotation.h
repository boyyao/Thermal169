#ifndef ROTATION_H
#define ROTATION_H

// 定义旋转类型
typedef enum {
    ROTATION_NONE,
    ROTATION_90_CW,
    ROTATION_90_CCW,
    ROTATION_180,
    FLIP_HORIZONTAL,
    FLIP_VERTICAL
} RotationType;

// 定义旋转类型
typedef enum {

    ROTATION_90_CW_TO_NEW

} Rotation_To_New_Type;



void rotate_90_cw_to_new(short unsigned input[32][32], short unsigned output[32][32]) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            output[j][31 - i] = input[i][j];
        }
    }
}



void rotate_90_clockwise_inplace(short unsigned array[32][32]) {
    // 分层旋转
    for (int layer = 0; layer < 16; layer++) {
        int first = layer;
        int last = 31 - layer;
        
        for (int i = first; i < last; i++) {
            int offset = i - first;
            
            // 保存上边
            short unsigned top = array[first][i];
            
            // 左边 → 上边
            array[first][i] = array[last - offset][first];
            
            // 下边 → 左边
            array[last - offset][first] = array[last][last - offset];
            
            // 右边 → 下边
            array[last][last - offset] = array[i][last];
            
            // 上边 → 右边
            array[i][last] = top;
        }
    }
}




void rotate_90_clockwise(short unsigned array[32][32]) {
    short unsigned temp[32][32];
    
    // 执行旋转
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            temp[j][31 - i] = array[i][j];
        }
    }
    
    // 复制回原数组
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            array[i][j] = temp[i][j];
        }
    }
}


void rotate_180(short unsigned array[32][32]) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            // 交换行对称元素
            short unsigned temp = array[i][j];
            array[i][j] = array[i][31 - j];
            array[i][31 - j] = temp;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            // 交换列对称元素
            short unsigned temp = array[i][j];
            array[i][j] = array[31 - i][j];
            array[31 - i][j] = temp;
        }
    }
}


void rotate_90_counterclockwise(short unsigned array[32][32]) {
    short unsigned temp[32][32];
    
    // 执行旋转
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            temp[31 - j][i] = array[i][j];
        }
    }
    
    // 复制回原数组
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            array[i][j] = temp[i][j];
        }
    }
}


// 旋转处理函数
void process_rotation(short unsigned data[32][32], RotationType rotation) {
    switch (rotation) {
        case ROTATION_90_CW:
            rotate_90_clockwise(data);
            break;
        case ROTATION_90_CCW:
            rotate_90_counterclockwise(data);
            break;
        case ROTATION_180:
            rotate_180(data);
            break;

        default:
            // 无旋转
            break;
    }
}

void process_rotation_to_new(short unsigned data[32][32],short unsigned newdata[32][32], Rotation_To_New_Type rotation) {
    switch (rotation) {
        case ROTATION_90_CW_TO_NEW:
            rotate_90_cw_to_new(data,newdata);
            break;
        default:
            // 无旋转
            break;
    }
}



#endif