#ifndef PIC_MANAGE_H
#define PIC_MANAGE_H

#include <Arduino.h>
#include <FreeRTOS.h>
#include <lvgl.h>
// #include "LittleFS.h"
#include <FatFS.h>
// #include <Adafruit_TinyUSB.h>
#include <FatFSUSB.h>
#include "Display.h"
#include "BilinearInterpolation.h"
#include "ui/actions.h"


volatile bool updated = false;
volatile bool driveConnected = false;
volatile bool inPrinting = false;

// Called by FatFSUSB when the drive is released.  We note this, restart FatFS, and tell the main loop to rescan.
// usb 拔出
void unplug(uint32_t i)
{
    (void)i;
    driveConnected = false;
    updated = true;
    FatFS.begin();
}

// Called by FatFSUSB when the drive is mounted by the PC.  Have to stop FatFS, since the drive data can change, note it, and continue.
// usb 插入
void plug(uint32_t i)
{
    (void)i;
    driveConnected = true;
    FatFS.end();
}

// Called by FatFSUSB to determine if it is safe to let the PC mount the USB drive.  If we're accessing the FS in any way, have any Files open, etc. then it's not safe to let the PC mount the drive.
bool mountable(uint32_t i)
{
    (void)i;
    return !inPrinting;
}

void printDirectory(String dirName, int numTabs)
{
    Dir dir = FatFS.openDir(dirName);
    while (true)
    {
        if (!dir.next())
        {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }
        Serial.print(dir.fileName());
        if (dir.isDirectory())
        {
            Serial.println("/");
            printDirectory(dirName + "/" + dir.fileName(), numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.print(dir.fileSize(), DEC);
            time_t cr = dir.fileCreationTime();
            struct tm *tmstruct = localtime(&cr);
            Serial.printf("\t%d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        }
    }
}

uint32_t fileCounter = 0;                      // 文件计数器
const char *COUNTER_FILE = "file_counter.dat"; // 存储计数器的文件

// 保存当前计数器值
void saveCounter()
{
    File counterFile = FatFS.open(COUNTER_FILE, "w");
    if (counterFile)
    {
        counterFile.write((uint8_t *)&fileCounter, sizeof(fileCounter));
        counterFile.close();
    }
}

// 生成唯一文件名
String generateUniqueFilename(const char *prefix, const char *extension)
{
    fileCounter++; // 增加计数器
    saveCounter(); // 立即保存计数器

    char filename[32];
    snprintf(filename, sizeof(filename), "%s_%04u.%s", prefix, fileCounter, extension);
    return String(filename);
}

void save_data_pixel()
{
    Serial.println("Saving pixel_data...");
    String filename = generateUniqueFilename("data", "csv");
    File file = FatFS.open(filename, "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    // 逐行写入数据
    for (int row = 0; row < 32; row++)
    {
        // 每行的第一个像素前不加逗号
        file.printf("%u", new_data_pixel[row][0]);
        for (int col = 1; col < 32; col++)
        {
            file.printf(",%u", new_data_pixel[row][col]); // 像素值
        }
        file.println(); // 换行
    }
    file.close(); // 关闭文件

    // 在生成一个JPG文件?
}

static uint16_t buf_raw_pic[96 * 96] __attribute__((aligned(4))); // Processed image data (raw colors)

static lv_img_dsc_t img_dsc;

String removeExtension(const String &filename)
{
    // 查找最后一个点号的位置
    int dotIndex = filename.lastIndexOf('.');

    // 如果没有点号或点号在开头（隐藏文件），返回原字符串
    if (dotIndex <= 0)
    {
        return filename;
    }

    // 返回点号前的子字符串
    return filename.substring(0, dotIndex);
}

// 文件名点击。打开缩略图
static void event_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    const char *filename = lv_list_get_button_text(objects.list_file, obj);
    //Serial.println(filename);
    lv_label_set_text_fmt(objects.lab_filename, "%s.csv", filename);

    File picdata = FatFS.open(lv_label_get_text(objects.lab_filename), "r");
    // 读取每行数据并填充数组
    for (int row = 0; row < 32; row++)
    {
        String line = picdata.readStringUntil('\n'); // 读取一行数据
        int col = 0;
        int startIndex = 0;
        for (int i = 0; i < line.length(); i++)
        {
            // 找到每个逗号
            if (line.charAt(i) == ',' || i == line.length() - 1)
            {
                String valueStr = line.substring(startIndex, i); // 提取数字部分
                new_data_pixel[row][col] = valueStr.toInt();     // 转换为整数并存入数组
                col++;                                           // 移动到下一列
                startIndex = i + 1;                              // 更新下一个数字的起始位置
            }
        }
    }
    // 关闭文件
    picdata.close();

    short unsigned min_val = new_data_pixel[0][0]; // Start with the first element
    short unsigned max_val = new_data_pixel[0][0]; // Start with the first element

    // Loop through the 2D array
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            short unsigned value = new_data_pixel[i][j];
            // Check for min value
            if (value < min_val)
            {
                min_val = value;
            }
            // Check for max value
            if (value > max_val)
            {
                max_val = value;
            }
        }
    }

    data_pixel_to_draw_pixel_in_pic(new_data_pixel, max_val, min_val); // 将新数据转换为绘图数据

    unsigned short value;
    for (int y = 0; y < 32 * 3; y++)
    {
        for (int x = 0; x < 32 * 3; x++)
        {
            value = bio_interpolate(RES_96x96, interpolation_method, y, x, draw_pixel);
            buf_raw_pic[x + y * (32 * 3)] = colormap[value];
        }
    }

    // 初始化图像描述符
    memset(&img_dsc, 0, sizeof(img_dsc));
    img_dsc.header.w = 96;
    img_dsc.header.h = 96;
    img_dsc.data_size = 96 * 96 * sizeof(uint16_t);
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB565;
    img_dsc.data = (const uint8_t *)buf_raw_pic;

    // 设置图像源并刷新
    lv_img_set_src(objects.image_pic, &img_dsc);
    // lv_obj_invalidate(objects.image_pic);
}

void load_datalist()
{
    lv_obj_clean(objects.list_file); // 删除所有子对象



    Dir dir = FatFS.openDir("/");
    lv_obj_t *btn;
    while (dir.next())
    {
        String filename = dir.fileName();
        if (filename.endsWith(".csv"))
        {
            Serial.println("Found data file: " + filename);
            btn = lv_list_add_button(objects.list_file, NULL, removeExtension(filename).c_str());
            lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
        }
    }
}

void action_but_del_button(lv_event_t *e) {
    if(FatFS.remove(lv_label_get_text(objects.lab_filename)))
    {
        Serial.printf("Deleted file: %s\n", lv_label_get_text(objects.lab_filename));
        lv_label_set_text(objects.lab_filename, ""); // 清空标签
        load_datalist(); // 重新加载文件列表
    }
    else
    {
        Serial.println("Failed to delete file.");
    }
}




void usbFS_loop()
{
    if (updated && !driveConnected)
    {
        inPrinting = true;
        // Serial.println("\n\nDisconnected, new file listing:");
        printDirectory("/", 0);
        updated = false;
        inPrinting = false;
    }
}

void FATFS_init()
{
    FatFSConfig config;
    config.setUseFTL(false);   // 直接访问闪存
    config.setDirEntries(256); // 根目录条目数
    config.setFATCopies(1);    // 仅保留1份FAT表
    FatFS.setConfig(config);

    // 初始化并格式化文件系统
    bool fsMounted = FatFS.begin();
    if (!fsMounted)
    {
        Serial.println("Formatting FAT partition...");
        FatFS.format();            // 使用配置进行格式化
        fsMounted = FatFS.begin(); // 格式化后重新挂载
    }
    if (fsMounted)
        Serial.println("FS initialization done.");
    else
        Serial.println("FS initialization failed!");

    // 尝试读取计数器文件
    File counterFile = FatFS.open(COUNTER_FILE, "r");
    if (counterFile)
    {
        counterFile.read((uint8_t *)&fileCounter, sizeof(fileCounter));
        counterFile.close();
        Serial.printf("读取文件计数器: %u\n", fileCounter);
    }
    else
    {
        Serial.println("未找到计数器文件，从零开始");
        fileCounter = 0;
        saveCounter(); // 创建初始计数器文件
    }

    inPrinting = true;
    printDirectory("/", 0);
    inPrinting = false;

    // Set up callbacks
    FatFSUSB.onUnplug(unplug);
    FatFSUSB.onPlug(plug);
    FatFSUSB.driveReady(mountable);
    // Start FatFS USB drive mode
    FatFSUSB.begin();
    // Serial.println("FatFSUSB started.");
    // Serial.println("Connect drive via USB to upload/erase files and re-display");

    // 配置 USB MSC
    // usb_msc.setID("RPI", "PicoFS", "1.0");  // 厂商ID, 产品ID, 版本
    // // usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
    // usb_msc.setCapacity(FS_SIZE / 512, 512);  // 扇区数, 扇区大小
    // usb_msc.setUnitReady(true);     // 设备就绪
    // usb_msc.begin();                // 启动 MSC
    // 初始化 TinyUSB
    // TinyUSB_Device_Init(0);         // 使用默认配置
}

#endif