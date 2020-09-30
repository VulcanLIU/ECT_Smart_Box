# 1 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
# 2 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 3 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 4 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 5 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 6 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 7 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 8 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 9 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 10 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 11 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 12 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 34 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
//按键的行引脚队列
uint8_t KEY_ROW_PIN_ARRY[4] = {A7, A6, A5, A4};
//按键的列引脚队列
uint8_t KEY_COLUMN_PIN_ARRY[4] = {A3, A2, A1, A0};
//按键值对照表
char KEY_VALUE_ARRY[4][4] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', 'A', 'B'},
    {'C', 'D', 'E', 'F'}};

//步进电机引脚数组
uint8_t STEPPER_PIN_ARRY[4 /**/][4] = {
    {50, 51, 52, 53},
    {46, 47, 48, 49},
    {42, 43, 44, 45},
    {38, 39, 40, 41}};

//队列
QueueHandle_t xKeyPadQueue; //键盘的消息队列
QueueHandle_t xDisplayIndexQueue; //用于显示器显示内容的指针队列

//信号量
SemaphoreHandle_t xMutex;

//主逻辑任务句柄
TaskHandle_t xTaskMainLogicHandle = 
# 60 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
                                   __null
# 60 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                       ;

//定时器任务句柄
TaskHandle_t xTaskAlarm1Handle = 
# 63 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
                                __null
# 63 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                    ;
TaskHandle_t xTaskAlarm2Handle = 
# 64 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
                                __null
# 64 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                    ;
TaskHandle_t xTaskAlarm3Handle = 
# 65 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
                                __null
# 65 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                    ;
TaskHandle_t xTaskAlarm4Handle = 
# 66 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
                                __null
# 66 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                    ;

//定时器任务句柄数组
TaskHandle_t xTaskAlarmHandle[4] = {
    xTaskAlarm1Handle,
    xTaskAlarm2Handle,
    xTaskAlarm3Handle,
    xTaskAlarm4Handle};

//初始化键盘
Keypad customKeypad = Keypad(((char*)KEY_VALUE_ARRY), KEY_ROW_PIN_ARRY, KEY_COLUMN_PIN_ARRY, 4, 4);

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

PAGE Page[5 /*页面总数*/] = {
    {
        {{0, 1, " "}, {8, 1, " "}}, // UI界面文字
        {{0, 0, 0}, {5, 0, 0}, {10, 0, 0}} //
    }, //Page0;
    {
        {{0, 1, "Alarm1"}, {8, 1, "B"}}, // UI界面文字
        {{9, 1, 0}, {12, 1, 0}, {15, 1, 0}} //小时/分钟/药量
    }, //Page1;
    {
        {{0, 1, "Alarm2"}, {8, 1, "D"}}, // UI界面文字
        {{9, 1, 0}, {12, 1, 0}, {15, 1, 0}} //小时/分钟/药量
    }, //Page2;
    {
        {{0, 1, "Alarm3"}, {8, 1, "B"}}, // UI界面文字
        {{9, 1, 0}, {12, 1, 0}, {15, 1, 0}} //小时/分钟/药量
    }, //Page3;
    {
        {{0, 1, "Alarm4"}, {8, 1, "B"}}, // UI界面文字
        {{9, 1, 0}, {12, 1, 0}, {15, 1, 0}} //小时/分钟/药量
    } //Page4;
};

typedef struct DISPLAYINDEX
{
    uint8_t F_key_num; //功能按键次数
    uint8_t F_key_MAX_num = 3 /*每页数据量*/ * 5 /*页面总数*/; //功能按键最大按键次数
    uint8_t page; //页数
    uint8_t index; //光标位置
    bool index_visiable; //光标是否显示
};

//给定时器给出数字标号
int Alarm1_index = 1;
int Alarm2_index = 2;
int Alarm3_index = 3;
int Alarm4_index = 4;

//实例化 DS1302对象
ThreeWire myWire(4, 5, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

//标记现在时间
RtcDateTime now;

//标记四个闹钟时间
RtcDateTime alarm[4];

//任务函数列表
void TaskKeyboardRead(void *pvParameters); //键盘读取任务
void TaskMainLogic(void *pvParameters); //主逻辑任务
void TaskDisplay(void *pvParameters); //1602显示器显示任务
void TaskAlarm(void *pvParameters); //报警

void setup()
{
    //打开串口
    Serial.begin(115200);

    //读取数据
    for (uint8_t i = 0; i < 5 /*页面总数*/; i++)
    {
        for (uint8_t j = 0; j < 3 /*每页数据量*/; j++)
        {
            Page[i].UI_data[j].data = EEPROM.read(i * 3 /*每页数据量*/ + j);
        }
    }

    //启动显示器
    lcd.begin(16, 2);
    lcd.clear();

    //启动四个指示灯
    Alarm1_LED.init();
    Alarm2_LED.init();
    Alarm3_LED.init();
    Alarm4_LED.init();

    //启动DS1302
    Rtc.Begin();

    //蜂鸣器启动
    BUZZER_init();

    //启动光电门
    Photogate_init();

    Serial.print("compiled: ");
    Serial.print("Sep 30 2020");
    Serial.println("11:17:54");

    delay(500);

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }
    //获取编译器时间并写入DS1302
    RtcDateTime compiled = RtcDateTime("Sep 30 2020", "11:17:54");
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled)
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled)
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled)
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    //创建按键队列-10个值,char类型变量
    xKeyPadQueue = xQueueGenericCreate( ( 10 ), ( sizeof(char) ), ( ( ( uint8_t ) 0U ) ) );

    //创建显示内容队列
    xDisplayIndexQueue = xQueueGenericCreate( ( 3 ), ( sizeof(DISPLAYINDEX) ), ( ( ( uint8_t ) 0U ) ) );

    //创建互斥信号量
    xMutex = xQueueCreateMutex( ( ( uint8_t ) 1U ) );

    //创建键盘读取任务
    xTaskCreate(TaskKeyboardRead, //任务函数
                "KeyboardRead", //任务名称
                128, //堆栈深度
                
# 235 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 235 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   , //
                4, //优先级
                
# 237 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 237 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   ); //传入参数

    //创建主逻辑任务
    xTaskCreate(TaskMainLogic,
                "MainLogic",
                128,
                
# 243 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 243 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   ,
                3,
                &xTaskMainLogicHandle);

    //创建显示器显示任务
    xTaskCreate(TaskDisplay,
                "Display",
                128,
                
# 251 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 251 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   ,
                5,
                
# 253 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 253 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   );

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
# 274 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
            xStatus = xQueueGenericSend( ( xKeyPadQueue ), ( &customKey ), ( ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 287 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                     0 
# 287 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                     /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ) ), ( ( BaseType_t ) 0 ) );

            Serial.println(customKey);

            //判断写入状态
            if (xStatus != ( ( ( BaseType_t ) 1 ) ))
            {
                Serial.println("ERROR!");
            }
        }

        //任务延时
        vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 299 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  0 
# 299 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                  /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
    }
}

/**

 * @brief 主逻辑任务

 * 

 * @param pvParameters 

 */
# 308 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
        xStatus = xQueueReceive(xKeyPadQueue, lReceivedValue, ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 323 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                                                             0 
# 323 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                                             /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));

        if (xStatus == ( ( ( BaseType_t ) 1 ) ))
        {
            Serial.println(lReceivedValue[0]);

            BUZZER_SetHigh();

            vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 331 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                      0 
# 331 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                      /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));

            BUZZER_SetLow();

            //任务管理

            //*根据按键内容更改显示内容
            switch (lReceivedValue[0])
            {
            case '0': //功能键
                if (lDisplayIndex.F_key_num < lDisplayIndex.F_key_MAX_num) //功能按键次数大于最大次数
                {
                    //如果第一页
                    if (lDisplayIndex.page == 0)
                    {
                        lDisplayIndex.F_key_num = 3 /*每页数据量*/;
                    }

                    //显示屏光标可见
                    lDisplayIndex.index_visiable = true;

                    //计算当前页面
                    lDisplayIndex.page = lDisplayIndex.F_key_num / 3 /*每页数据量*/;

                    //计算当前页面指向内容
                    lDisplayIndex.index = lDisplayIndex.F_key_num - 3 /*每页数据量*/ * lDisplayIndex.page;

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
                    for (uint8_t i = 0; i < 5 /*页面总数*/; i++)
                    {
                        for (uint8_t j = 0; j < 3 /*每页数据量*/; j++)
                        {
                            EEPROM.write(i * 3 /*每页数据量*/ + j, Page[i].UI_data[j].data);
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
                    if (_temp >= 24 && lDisplayIndex.index == 0)
                        _temp -= 24;
                    if (_temp >= 60 && lDisplayIndex.index == 1)
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
                        _temp += 23;
                    if (_temp <= 0 && lDisplayIndex.index == 1)
                        _temp += 59;
                    if (_temp <= 0 && lDisplayIndex.index == 2)
                        _temp += 99;
                    Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].data = _temp;
                }
                break;
            case '4': //药仓1定时按钮
                Alarm1_LED.update();

                //创建定时任务
                if (Alarm1_LED.state())
                {
                    //

                    //创建任务
                    xTaskCreate(TaskAlarm,
                                "TaskAlarm1",
                                1000,
                                Alarm1_index,
                                2,
                                &xTaskAlarm1Handle);
                }
                else
                {
                    //删除任务
                    vTaskDelete(xTaskAlarm1Handle);
                }

                break;
            case '5': //药仓2定时按钮
                Alarm2_LED.update();

                //创建定时任务
                if (Alarm2_LED.state())
                {
                    //创建任务
                    xTaskCreate(TaskAlarm,
                                "TaskAlarm2",
                                1000,
                                Alarm2_index,
                                2,
                                &xTaskAlarm2Handle);
                }
                else
                {
                    //删除任务
                    vTaskDelete(xTaskAlarm2Handle);
                }

                break;
            case '6': //药仓3定时按钮
                Alarm3_LED.update();

                //创建定时任务
                if (Alarm3_LED.state())
                {
                    //创建任务
                    xTaskCreate(TaskAlarm,
                                "TaskAlarm3",
                                1000,
                                Alarm3_index,
                                2,
                                &xTaskAlarm3Handle);
                }
                else
                {
                    //删除任务
                    vTaskDelete(xTaskAlarm3Handle);
                }
                break;
            case '7': //药仓4定时按钮
                Alarm4_LED.update();

                //创建定时任务
                if (Alarm4_LED.state())
                {
                    //创建任务
                    xTaskCreate(TaskAlarm,
                                "TaskAlarm4",
                                1000,
                                Alarm4_index,
                                2,
                                &xTaskAlarm4Handle);
                }
                else
                {
                    //删除任务
                    vTaskDelete(xTaskAlarm4Handle);
                }
                break;
            default:
                break;
            }
        }
        //*显示内容放入队列
        xStatus = xQueueGenericSend( ( xDisplayIndexQueue ), ( &lDisplayIndex ), ( ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 520 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                 0 
# 520 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                 /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ) ), ( ( BaseType_t ) 0 ) );

        if (xStatus == ( ( ( BaseType_t ) 1 ) ))
        {
        }
        else
        {

            //Serial.println("ERROR CODE: 002");
            DISPLAYINDEX lDisplayIndex_;

            //读取头部数据
            xQueueReceive(xDisplayIndexQueue, &lDisplayIndex_, ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 532 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                                                              0 
# 532 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                                              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));

            //再次尝试把数据放入队列
            xStatus = xQueueGenericSend( ( xDisplayIndexQueue ), ( &lDisplayIndex ), ( ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 535 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                     0 
# 535 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                     /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ) ), ( ( BaseType_t ) 0 ) );

            if (xStatus == ( ( ( BaseType_t ) 1 ) ))
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
    vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 549 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
              0 
# 549 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
}

/**

 * @brief 显示器显示任务

 * 

 * @param pvParameters 

 */
# 557 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
void TaskDisplay(void *pvParameters)
{
    //标识队列读取状态
    BaseType_t xStatus;

    //显示屏显示内容
    DISPLAYINDEX lDisplayIndex;

    while (1)
    {
        //接收显示队列数据
        xStatus = xQueueReceive(xDisplayIndexQueue, &lDisplayIndex, ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 568 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                                                                   0 
# 568 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                                                   /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
        if (xStatus == ( ( ( BaseType_t ) 1 ) ))
        {
        }
        else
        {
            Serial.println("ERROR CODE: 003");
        }

        //进入临界区
        __asm__ __volatile__ ( "in __tmp_reg__, __SREG__" "\n\t" "cli" "\n\t" "push __tmp_reg__" "\n\t" ::: "memory" );

        char datestring[30] = {};

        //读取当前时间
        now = Rtc.GetDateTime();
        snprintf_P(datestring,
                   (sizeof(datestring) / sizeof(datestring[0])),
                   
# 586 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 586 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                  "%02u/%02u %02u:%02u:%02u"
# 586 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  ); &__c[0];}))
# 586 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                                  ,
                   now.Month(),
                   now.Day(),
                   now.Hour(),
                   now.Minute(),
                   now.Second());

        // RtcDateTime l = RtcDateTime(l_y,l_m,l_d,l_h,l_mi,l_s); 

        // if(now > l)
        // {
        //     vTaskDelete(xTaskMainLogicHandle);
        // }

        // Serial.println(datestring);

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
            lcd.setCursor(8, //X位置
                          1); //Y位置

            char datestring2[10] = {};

            //将定时器数据内容组合为字符串
            snprintf_P(datestring2,
                       (sizeof(datestring2) / sizeof(datestring2[0])),
                       
# 628 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                      (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 628 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                      "%02u:%02u %02u"
# 628 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                      ); &__c[0];}))
# 628 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                            ,
                       Page[lDisplayIndex.page].UI_data[0].data,
                       Page[lDisplayIndex.page].UI_data[1].data,
                       Page[lDisplayIndex.page].UI_data[2].data);

            lcd.print(datestring2); //显示数据

            //*显示光标
            lcd.setCursor(Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].X, //X位置
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
        __asm__ __volatile__ ( "pop __tmp_reg__" "\n\t" "out __SREG__, __tmp_reg__" "\n\t" ::: "memory" );

        vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 300 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 655 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  0 
# 655 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                  /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
    }
}

void TaskAlarm(void *pvParameters)
{
    uint8_t _day = 0;
    int Alarm_index = (int *)pvParameters;
    //初始化步进电机
    const uint16_t STEPS = 100;
    Stepper myStepper(STEPS,
                      STEPPER_PIN_ARRY[Alarm_index - 1][0],
                      STEPPER_PIN_ARRY[Alarm_index - 1][1],
                      STEPPER_PIN_ARRY[Alarm_index - 1][2],
                      STEPPER_PIN_ARRY[Alarm_index - 1][3]);

    //*设置步进电机速度
    myStepper.setSpeed(200);

    //记录定时时间-初始化为RtcDataTime对象
    RtcDateTime alarm_time = RtcDateTime(now.Year(),
                                         now.Month(),
                                         now.Day(),
                                         Page[Alarm_index].UI_data[0].data,
                                         Page[Alarm_index].UI_data[1].data,
                                         0);

    Pill_left[Alarm_index - 1] = Page[Alarm_index].UI_data[2].data;

    //DEBUG调试数据输出区
    Serial.print("Alarm_index:    ");
    Serial.print(Alarm_index);

    Serial.print("  PIN:    ");
    Serial.print(STEPPER_PIN_ARRY[Alarm_index - 1][0]);

    Serial.print("  Alarm Hour:    ");
    Serial.print(alarm_time.Hour());

    Serial.print("  Alarm Minute:    ");
    Serial.println(alarm_time.Minute());

    while (1)
    {
        //判断定时
        if (alarm_time.Hour() == now.Hour() && alarm_time.Minute() == now.Minute() && _day != now.Day()) //alarm_time.Hour() == now.Hour() && alarm_time.Minute() == now.Minute()
        {
            //保存最后提醒时间
            _day = now.Day();

            //尝试获取互斥信号量
            xQueueSemaphoreTake( ( xMutex ), ( ( ( TickType_t ) ( ( ( uint32_t ) ( 10000 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 706 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
           0 
# 706 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
           /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ) ) );
            {
                //光电门中断
                attachInterrupt(((Photogate_PIN_ARRY[Alarm_index - 1]) == 2 ? 0 : ((Photogate_PIN_ARRY[Alarm_index - 1]) == 3 ? 1 : ((Photogate_PIN_ARRY[Alarm_index - 1]) >= 18 && (Photogate_PIN_ARRY[Alarm_index - 1]) <= 21 ? 23 - (Photogate_PIN_ARRY[Alarm_index - 1]) : -1))),
                                Photogate_ISR[Alarm_index - 1],
                                3);

                //先出药
                while (Pill_left[Alarm_index - 1] > 0)
                {
                    Serial.print("Pill_left:    ");
                    Serial.println(Pill_left[Alarm_index - 1]);
                    myStepper.step(STEPS);
                }

                //再响铃
                for (uint8_t i = 0; i < 10; i++)
                {
                    //
                    BUZZER_SetHigh();
                    vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 90 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 726 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                              0 
# 726 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
                    BUZZER_SetLow();
                    vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 90 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 728 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                              0 
# 728 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));

                    //
                    BUZZER_SetHigh();
                    vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 90 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 732 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                              0 
# 732 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
                    BUZZER_SetLow();
                    vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 600 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 734 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                              0 
# 734 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
                }
            }
            xQueueGenericSend( ( QueueHandle_t ) ( xMutex ), 
# 737 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
           __null
# 737 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
           , ( ( TickType_t ) 0U ), ( ( BaseType_t ) 0 ) );

            //取消中断
            detachInterrupt(((Photogate_PIN_ARRY[Alarm_index - 1]) == 2 ? 0 : ((Photogate_PIN_ARRY[Alarm_index - 1]) == 3 ? 1 : ((Photogate_PIN_ARRY[Alarm_index - 1]) >= 18 && (Photogate_PIN_ARRY[Alarm_index - 1]) <= 21 ? 23 - (Photogate_PIN_ARRY[Alarm_index - 1]) : -1))));

            //还原药量
            Pill_left[Alarm_index - 1] = Page[Alarm_index].UI_data[2].data;
        }
        vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 2000 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 745 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  0 
# 745 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                  /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
    }
}
