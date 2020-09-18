#include "Arduino_FreeRTOS.h"
#include "semphr.h" // add the FreeRTOS functions for Semaphores (or Flags).
#include <Keypad.h>
#include <LiquidCrystal.h>
#include "ThreeWire.h"
#include "RtcDS1302.h"
#include <EEPROM.h>

#define KEY_COLUMN_NUM 4
#define KEY_ROW_NUM 4

#define KEY_ROW0_PIN A7
#define KEY_ROW1_PIN A6
#define KEY_ROW2_PIN A5
#define KEY_ROW3_PIN A4
#define KEY_COLUMN0_PIN A3
#define KEY_COLUMN1_PIN A2
#define KEY_COLUMN2_PIN A1
#define KEY_COLUMN3_PIN A0

#define BUZZER_P_PIN 22
#define BUZZER_N_PIN 26

#define ALARM1_LED_P_PIN 34
#define ALARM1_LED_N_PIN 35
#define ALARM2_LED_P_PIN 38

#define PAGE_DATA_NUM 3                        //每页数据量
#define PAGE_NUM 5                             //页面总数
#define DATA_TOTAL_NUM PAGE_DATA_NUM *PAGE_NUM //需要保存的数据总量

#define countof(a) (sizeof(a) / sizeof(a[0]))

//按键的行引脚队列
uint8_t KEY_ROW_PIN_ARRY[KEY_ROW_NUM] = {KEY_ROW0_PIN, KEY_ROW1_PIN, KEY_ROW2_PIN, KEY_ROW3_PIN};
//按键的列引脚队列
uint8_t KEY_COLUMN_PIN_ARRY[KEY_COLUMN_NUM] = {KEY_COLUMN0_PIN, KEY_COLUMN1_PIN, KEY_COLUMN2_PIN, KEY_COLUMN3_PIN};
//按键值对照表
char KEY_VALUE_ARRY[KEY_ROW_NUM][KEY_COLUMN_NUM] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', 'A', 'B'},
    {'C', 'D', 'E', 'F'}};

//队列
QueueHandle_t xKeyPadQueue;       //键盘的消息队列
QueueHandle_t xDisplayIndexQueue; //用于显示器显示内容的指针队列

//信号量
SemaphoreHandle_t xSerialSemaphore;

//初始化键盘
Keypad customKeypad = Keypad(makeKeymap(KEY_VALUE_ARRY), KEY_ROW_PIN_ARRY, KEY_COLUMN_PIN_ARRY, KEY_ROW_NUM, KEY_COLUMN_NUM);

//初始化显示器
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

//显示UI
String lcd_UI[] = {};

//UI文字
typedef struct UI_STR
{
    uint8_t X;
    uint8_t Y;
    String string;
};

//UI数据
typedef struct UI_DATA
{
    uint8_t X;
    uint8_t Y;
    uint8_t data;
};

//每页UI内容
typedef struct PAGE
{
    UI_STR UI_str[2];
    UI_DATA UI_data[3]; //每页包含 小时/分钟/药量 三个数据数据;
};

PAGE Page[PAGE_NUM] = {
    {
        {{0, 1, " "}, {8, 1, " "}},           // UI界面文字
        {{0, 1, 12}, {5, 1, 12}, {10, 1, 14}} //
    },
    {
        {{0, 1, "Alarm1"}, {8, 1, "B"}},       // UI界面文字
        {{9, 1, 12}, {12, 1, 12}, {15, 1, 14}} //
    },                                         //Page1;
    {
        {{0, 1, "Alarm2"}, {8, 1, "D"}},       // UI界面文字
        {{9, 1, 12}, {12, 1, 12}, {15, 1, 14}} //
    },                                         //Page2;
    {
        {{0, 1, "Alarm3"}, {8, 1, "B"}},       // UI界面文字
        {{9, 1, 12}, {12, 1, 12}, {15, 1, 14}} //
    },                                         //Page3;
    {
        {{0, 1, "Alarm4"}, {8, 1, "B"}},       // UI界面文字
        {{9, 1, 12}, {12, 1, 12}, {15, 1, 14}} //
    }                                          //Page4;
};

typedef struct DISPLAYINDEX
{
    uint8_t F_key_num;                                //功能按键次数
    uint8_t F_key_MAX_num = PAGE_DATA_NUM * PAGE_NUM; //功能按键最大按键次数
    uint8_t page;                                     //页数
    uint8_t index;                                    //光标位置
    bool index_visiable;                              //光标是否显示
};

//实例化 DS1302对象
ThreeWire myWire(4, 5, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

//标记现在时间
RtcDateTime now;

//标记四个闹钟时间
RtcDateTime alarm[4];

//任务函数列表
void TaskKeyboardRead(void *pvParameters); //键盘读取任务
void TaskMainLogic(void *pvParameters);    //主逻辑任务
void TaskDisplay(void *pvParameters);      //1602显示器显示任务
void TaskAlarm(void *pvParameters);        //报警

void setup()
{
    //打开串口
    Serial.begin(115200);

    //读取数据
    for (uint8_t i = 0; i < PAGE_NUM; i++)
    {
        for (uint8_t j = 0; j < PAGE_DATA_NUM; j++)
        {
            Page[i].UI_data[j].data = EEPROM.read(i * PAGE_DATA_NUM + j);
        }
    }

    //启动显示器
    lcd.begin(16, 2);

    //启动DS1302
    Rtc.Begin();

    //蜂鸣器启动
    BUZZER_init();

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    delay(500);

    //获取编译器时间并写入DS1302
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(compiled);

    //创建按键队列-10个值,char类型变量
    xKeyPadQueue = xQueueCreate(10, sizeof(char));

    //创建显示内容队列
    xDisplayIndexQueue = xQueueCreate(3, sizeof(DISPLAYINDEX));

    //创建键盘读取任务
    xTaskCreate(TaskKeyboardRead, //任务函数
                "KeyboardRead",   //任务名称
                128,              //堆栈深度
                NULL,             //
                4,                //优先级
                NULL);            //传入参数

    //创建主逻辑任务
    xTaskCreate(TaskMainLogic,
                "MainLogic",
                128,
                NULL,
                3,
                NULL);

    //创建显示器显示任务
    xTaskCreate(TaskDisplay,
                "Display",
                128,
                NULL,
                5,
                NULL);

    //启动调度器
    vTaskStartScheduler();
}

void loop()
{
    //空任务
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

/**键盘读取程序
 * @brief 
 * 
 * @param pvParameters 
 */

void TaskKeyboardRead(void *pvParameters)
{
    //标识队列发送状态
    BaseType_t xStatus;

    while (1)
    {
        //读取按键
        char customKey = customKeypad.getKey();

        //键值写入队列
        if (customKey)
        {
            xStatus = xQueueSendToBack(xKeyPadQueue, &customKey, pdMS_TO_TICKS(30));

            Serial.println(customKey);

            //判断写入状态
            if (xStatus != pdPASS)
            {
                Serial.println("ERROR!");
            }
        }

        //任务延时
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

/**
 * @brief 主逻辑任务
 * 
 * @param pvParameters 
 */
void TaskMainLogic(void *pvParameters)
{
    //读取的按键值
    char lReceivedValue[] = {};

    //显示屏显示内容
    DISPLAYINDEX lDisplayIndex = {};

    //标识队列读取状态
    BaseType_t xStatus;

    while (1)
    {
        //读取按键
        //*按键内容送入队列
        xStatus = xQueueReceive(xKeyPadQueue, lReceivedValue, pdMS_TO_TICKS(30));

        if (xStatus == pdPASS)
        {
            Serial.println(lReceivedValue[0]);

            BUZZER_SetHigh();

            vTaskDelay(pdMS_TO_TICKS(30));

            BUZZER_SetLow();

            //任务管理

            //*根据按键内容更改显示内容
            switch (lReceivedValue[0])
            {
            case '0':                                                      //功能键
                if (lDisplayIndex.F_key_num < lDisplayIndex.F_key_MAX_num) //功能按键次数大于最大次数
                {
                    //如果第一页
                    if (lDisplayIndex.page == 0)
                    {
                        lDisplayIndex.F_key_num = PAGE_DATA_NUM;
                    }

                    //显示屏光标可见
                    lDisplayIndex.index_visiable = true;

                    //计算当前页面
                    lDisplayIndex.page = lDisplayIndex.F_key_num / PAGE_DATA_NUM;

                    //计算当前页面指向内容
                    lDisplayIndex.index = lDisplayIndex.F_key_num - PAGE_DATA_NUM * lDisplayIndex.page;

                    //**功能键次数++
                    lDisplayIndex.F_key_num++;

                    Serial.print("  F_key_num:");
                    Serial.println(lDisplayIndex.F_key_num);
                }
                else
                {
                    //按键次数归零
                    lDisplayIndex.F_key_num = 0;

                    //显示屏光标不可见
                    lDisplayIndex.index_visiable = false;

                    //显示屏page归零.index归零
                    lDisplayIndex.index = 0;
                    lDisplayIndex.page = 0;

                    //保存数据
                    for (uint8_t i = 0; i < PAGE_NUM; i++)
                    {
                        for (uint8_t j = 0; j < PAGE_DATA_NUM; j++)
                        {
                            EEPROM.write(i * PAGE_DATA_NUM + j, Page[i].UI_data[j].data);
                        }
                    }
                }
                Serial.print("page:");
                Serial.print(lDisplayIndex.page);
                Serial.print("  index:");
                Serial.print(lDisplayIndex.index);
                Serial.print("  F_key_num:");
                Serial.println(lDisplayIndex.F_key_num);

                break;

            case '1': //调节上键
                //如果第一页
                if (lDisplayIndex.page == 0)
                {
                }
                else
                {
                    int8_t _temp = Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].data;
                    _temp++;
                    if (_temp > 12 && lDisplayIndex.index == 0)
                        _temp -= 12;
                    if (_temp > 60 && lDisplayIndex.index == 1)
                        _temp -= 60;
                    if (_temp > 99 && lDisplayIndex.index == 2)
                        _temp -= 99;
                    Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].data = _temp;
                }

                break;
            case '2': //调节下键
                //如果第一页
                if (lDisplayIndex.page == 0)
                {
                }
                else
                {
                    int8_t _temp = Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].data;
                    _temp--;
                    if (_temp <= 0 && lDisplayIndex.index == 0)
                        _temp += 12;
                    if (_temp <= 0 && lDisplayIndex.index == 1)
                        _temp += 60;
                    if (_temp <= 0 && lDisplayIndex.index == 2)
                        _temp += 99;
                    Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].data = _temp;
                }
            case '4':

            default:
                break;
            }
        }
        //*显示内容放入队列
        xStatus = xQueueSendToBack(xDisplayIndexQueue, &lDisplayIndex, pdMS_TO_TICKS(30));

        if (xStatus == pdPASS)
        {
        }
        else
        {

            //Serial.println("ERROR CODE: 002");
            DISPLAYINDEX lDisplayIndex_;

            //读取头部数据
            xQueueReceive(xDisplayIndexQueue, &lDisplayIndex_, pdMS_TO_TICKS(30));

            //再次尝试把数据放入队列
            xStatus = xQueueSendToBack(xDisplayIndexQueue, &lDisplayIndex, pdMS_TO_TICKS(30));

            if (xStatus == pdPASS)
            {
                //Serial.println("ERROR SOLVED");
            }
            else
            {
                //Serial.println("ERROR CODE: 003");
            }
        }
    }

    //任务延时
    vTaskDelay(pdMS_TO_TICKS(30));
}

/**
 * @brief 显示器显示任务
 * 
 * @param pvParameters 
 */
void TaskDisplay(void *pvParameters)
{
    //标识队列读取状态
    BaseType_t xStatus;

    //显示屏显示内容
    DISPLAYINDEX lDisplayIndex;

    while (1)
    {
        //接收显示队列数据
        xStatus = xQueueReceive(xDisplayIndexQueue, &lDisplayIndex, pdMS_TO_TICKS(30));
        if (xStatus == pdPASS)
        {
        }
        else
        {
            Serial.println("ERROR CODE: 003");
        }

        //进入临界区
        taskENTER_CRITICAL();

        char datestring[30] = {};

        //读取当前时间
        now = Rtc.GetDateTime();
        snprintf_P(datestring,
                   countof(datestring),
                   PSTR("%02u/%02u %02u:%02u:%02u"),
                   now.Month(),
                   now.Day(),
                   now.Hour(),
                   now.Minute(),
                   now.Second());

        //Serial.println(datestring);

        //lcd清屏
        lcd.clear();

        //*lcd设置光标位置
        lcd.setCursor(0, 0);

        //*lcd显示时间
        lcd.print(datestring);

        //*lcd设置光标位置
        lcd.setCursor(Page[lDisplayIndex.page].UI_str->X, Page[lDisplayIndex.page].UI_str->Y);

        //*lcd显示UI文字内容
        lcd.print(Page[lDisplayIndex.page].UI_str->string);

        if (lDisplayIndex.page > 0)
        {
            //*lcd显示定时数据内容
            lcd.setCursor(8,  //X位置
                          1); //Y位置

            char datestring2[10] = {};

            //将定时器数据内容组合为字符串
            snprintf_P(datestring2,
                       countof(datestring2),
                       PSTR("%02u:%02u %02u"),
                       Page[lDisplayIndex.page].UI_data[0].data,
                       Page[lDisplayIndex.page].UI_data[1].data,
                       Page[lDisplayIndex.page].UI_data[2].data);

            lcd.print(datestring2); //显示数据

            //*显示光标
            lcd.setCursor(Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].X,  //X位置
                          Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].Y); //Y位置

            lcd.cursor();

            // Serial.print("page:");
            // Serial.print(lDisplayIndex.page);
            // Serial.print("  index:");
            // Serial.println(lDisplayIndex.index);
        }
        else
        {
            //隐藏光标
            lcd.noCursor();
        }

        //退出临界区
        taskEXIT_CRITICAL();

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

/**
 * @brief 蜂鸣器启动函数
 * 
 */
void BUZZER_init()
{
    pinMode(BUZZER_P_PIN, INPUT_PULLUP);
    pinMode(BUZZER_N_PIN, OUTPUT);
    digitalWrite(BUZZER_N_PIN, HIGH);
}

/**
 * @brief 蜂鸣器响
 * 
 */
void BUZZER_SetHigh()
{
    digitalWrite(BUZZER_N_PIN, LOW);
}

/**
 * @brief 蜂鸣器不响
 * 
 */
void BUZZER_SetLow()
{
    digitalWrite(BUZZER_N_PIN, HIGH);
}