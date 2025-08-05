#ifndef SHARED_VAL_H
#define SHARED_VAL_H

#include <FreeRTOS.h>
#include <task.h>
// class ScopedLock
// {
// public:
//     ScopedLock(bool &lockFlag) : m_lock(lockFlag)
//     {
//         m_lock = true; // 进入作用域时加锁
//     }

//     ~ScopedLock()
//     {
//         m_lock = false; // 离开作用域时自动解锁
//     }

// private:
//     bool &m_lock;
// };

class LockGuard
{
public:
    explicit LockGuard(volatile bool &lock) : lock_(lock)
    {
        lock_ = true;
    }

    ~LockGuard()
    {
        lock_ = false;
    }

    // 删除拷贝构造和赋值
    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;

private:
    volatile bool &lock_;
};

// 通用等待函数模板
template <typename T>
void waitForCondition(volatile T &condition, T target, uint32_t delay_ms = 5)
{
    while (condition != target)
    {
        delay(delay_ms);
    }
}

// 特化版本：等待bool为false
void waitForUnlock(volatile bool &lock, uint32_t delay_ms = 5)
{
    waitForCondition(lock, false, delay_ms);
}


void dump_memory_info()
{
    extern char __StackLimit, __StackTop, __HeapLimit, __end__;

    // 堆栈信息
    size_t stack_size = &__StackTop - &__StackLimit;
    size_t stack_used = &__StackTop - (char *)&stack_size;

    Serial.println("===== 内存信息 =====");
    Serial.print("堆栈起始: 0x");
    Serial.println((uintptr_t)&__StackLimit, HEX);
    Serial.print("堆栈结束: 0x");
    Serial.println((uintptr_t)&__StackTop, HEX);
    Serial.print("堆栈大小: ");
    Serial.print(stack_size);
    Serial.println(" 字节");
    Serial.print("堆栈使用: ");
    Serial.print(stack_used);
    Serial.println(" 字节");

    // 堆信息
    size_t heap_size = &__HeapLimit - &__end__;
    // size_t free_heap = get_free_heap_size();

    Serial.print("堆起始: 0x");
    Serial.println((uintptr_t)&__end__, HEX);
    Serial.print("堆结束: 0x");
    Serial.println((uintptr_t)&__HeapLimit, HEX);
    Serial.print("堆大小: ");
    Serial.print(heap_size);
    Serial.println(" 字节");
    // Serial.print("可用堆内存: ");
    // Serial.print(free_heap);
    // Serial.println(" 字节");
}




// 频率统计器结构体
struct CallFrequency {
  const char* name;           // 函数名称标识
  unsigned long callCount;    // 调用次数计数器
  unsigned long lastTime;     // 上次记录时间
  float currentFreq;          // 当前计算出的频率
};

// 初始化频率统计器
CallFrequency initFrequencyCounter(const char* funcName) {
  CallFrequency counter;
  counter.name = funcName;
  counter.callCount = 0;
  counter.lastTime = millis();
  counter.currentFreq = 0.0;
  return counter;
}

// 更新并显示调用频率
void updateFrequency(CallFrequency& counter, unsigned long reportInterval = 1000) {
  counter.callCount++;  // 增加调用计数
  
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - counter.lastTime;
  
  // 检查是否达到报告间隔
  if (elapsedTime >= reportInterval) {
    // 计算当前频率 (次/秒)
    counter.currentFreq = counter.callCount * 1000.0 / elapsedTime;
    
    // 串口输出结果
    Serial.print("Function '");
    Serial.print(counter.name);
    Serial.print("' called ");
    Serial.print(counter.callCount);
    Serial.print(" times in ");
    Serial.print(elapsedTime);
    Serial.print(" ms (");
    Serial.print(counter.currentFreq, 1);
    Serial.println(" Hz)");
    
    // 重置计数器和时间基准
    counter.callCount = 0;
    counter.lastTime = currentTime;
  }
}

/*
使用方法：
CallFrequency counter1 = initFrequencyCounter("DrawFunction"); //先声明
updateFrequency(counter1); //函数中调用
*/





//uint8_t cmap_now_choose = COLORMAP_CLASSIC; // 当前所使用的颜色映射表
// 线程锁
bool prob_lock = true;
bool pix_cp_lock = false;
bool cmap_loading_lock = false;      // 颜色映射表加载锁
bool PROB_READY = false; // 探头是否准备就绪
int brightness = 128;                // 屏幕亮度

uint16_t test_point[2] = {120, 120}; // 测温点的位置
bool flag_show_cursor = true;        // 是否显示温度采集指针
bool flag_show_temp_text = false;    // 是否显示温度文本
bool flag_trace_max = true;          // 是否使用最热点追踪
bool flag_trace_min = true; //低温追踪
int cursor_size = 10;                // 光标大小
int temp_text_size = 2;              // 温度文本大小


unsigned short T_max, T_min;         // 温度
unsigned long T_avg;                 // 需要累加后再除平均数。所以要long类型
uint8_t x_max, y_max, x_min, y_min; //最高温和最低温的点位


bool in_settings = false; // 是否在设置界面

bool flag_in_photo_mode = false; // 是否正处于照相模式(画面暂停)

bool flag_clear_cursor = false; // 是否拍照模式下清除温度采集指针

static short unsigned new_data_pixel[32][32];
unsigned short draw_pixel[PIXEL_PER_COLUMN][PIXEL_PER_ROW] = {0}; // 传感器数据转换成的像素数据

int interpolation_method = 3; // 0=最近邻, 1=1位双线性, 2=2位双线性, 3=4位双线性
int use_kalman_model = 2;     // 0=不使用，1=单次，2=多次
bool use_sfilter = true;      // 是否使用平滑滤波器

#define color_num 180              // 生成颜色数
#define color_num_f (static_cast<float>(color_num))

#define TFT_HOR_RES 240
#define TFT_VER_RES 280
#define TFT_ROTATION LV_DISPLAY_ROTATION_90
lv_disp_t *disp;
static TFT_eSPI tft = TFT_eSPI(TFT_HOR_RES, TFT_VER_RES);


#define SCREEN_BL_PIN 4
#define SCREEN_VDD 5





// 测试
static void mydelaytomain(void *ptr)
{
    // 延迟500ms
    vTaskDelay(500);
    // 任务完成后删除自身
    in_settings = false;
    vTaskDelete(NULL);
}
static void toMainScreen()
{
    loadScreen(SCREEN_ID_MAIN);
    xTaskCreate(
        mydelaytomain, // 任务函数指针
        "MyDelay",     // 任务名称（调试用）
        1024,          // 任务栈大小（字节）
        NULL,          // 传递给任务的参数
        5,             // 任务优先级（0-24）
        NULL           // 任务句柄指针
    );
}






#endif