# 1 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
# 2 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 3 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 4 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 5 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 6 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 7 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 2
# 28 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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

//队列
QueueHandle_t xKeyPadQueue; //键盘的消息队列
QueueHandle_t xDisplayIndexQueue; //用于显示器显示内容的指针队列

//信号量
SemaphoreHandle_t xSerialSemaphore;

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
        {{0, 1, 12}, {5, 1, 12}, {10, 1, 14}} //
    },
    {
        {{0, 1, "Alarm1"}, {8, 1, "B"}}, // UI界面文字
        {{8, 1, 12}, {5, 1, 12}, {10, 1, 14}} //
    }, //Page1;
    {
        {{0, 1, "Alarm2"}, {8, 1, "D"}}, // UI界面文字
        {{8, 1, 12}, {5, 1, 12}, {10, 1, 14}} //
    }, //Page2;
    {
        {{0, 1, "Alarm3"}, {8, 1, "B"}}, // UI界面文字
        {{8, 1, 12}, {5, 1, 12}, {10, 1, 14}} //
    }, //Page3;
    {
        {{0, 1, "Alarm4"}, {8, 1, "B"}}, // UI界面文字
        {{8, 1, 12}, {5, 1, 12}, {10, 1, 14}} //
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

    //启动显示器
    lcd.begin(16, 2);

    //启动DS1302
    Rtc.Begin();

    //蜂鸣器启动
    BUZZER_init();

    Serial.print("compiled: ");
    Serial.print("Sep 18 2020");
    Serial.println("22:44:13");
    delay(500);

    //获取编译器时间并写入DS1302
    RtcDateTime compiled = RtcDateTime("Sep 18 2020", "22:44:13");
    Rtc.SetDateTime(compiled);

    //创建按键队列-10个值,char类型变量
    xKeyPadQueue = xQueueGenericCreate( ( 10 ), ( sizeof(char) ), ( ( ( uint8_t ) 0U ) ) );

    //创建显示内容队列
    xDisplayIndexQueue = xQueueGenericCreate( ( 3 ), ( sizeof(DISPLAYINDEX) ), ( ( ( uint8_t ) 0U ) ) );

    //创建键盘读取任务
    xTaskCreate(TaskKeyboardRead, //任务函数
                "KeyboardRead", //任务名称
                128, //堆栈深度
                
# 159 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 159 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   , //
                4, //优先级
                
# 161 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 161 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   ); //传入参数

    //创建主逻辑任务
    xTaskCreate(TaskMainLogic,
                "MainLogic",
                128,
                
# 167 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 167 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   ,
                3,
                
# 169 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 169 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   );

    //创建显示器显示任务
    xTaskCreate(TaskDisplay,
                "Display",
                128,
                
# 175 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 175 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                   ,
                5,
                
# 177 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3 4
               __null
# 177 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
# 198 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
# 211 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                     0 
# 211 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
# 223 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  0 
# 223 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                  /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
    }
}

/**

 * @brief 主逻辑任务

 * 

 * @param pvParameters 

 */
# 232 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
# 247 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                                                             0 
# 247 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                                             /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));

        if (xStatus == ( ( ( BaseType_t ) 1 ) ))
        {
            Serial.println(lReceivedValue[0]);

            BUZZER_SetHigh();

            vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 255 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                      0 
# 255 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                      /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));

            BUZZER_SetLow();

            //任务管理
            //*根据按键内容更改显示内容

            switch (lReceivedValue[0])
            {
            case '1': //功能键
                if (lDisplayIndex.F_key_num < lDisplayIndex.F_key_MAX_num) //功能按键次数大于最大次数
                {
                    //第一页
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
                }
                Serial.print("page:");
                Serial.print(lDisplayIndex.page);
                Serial.print("  index:");
                Serial.print(lDisplayIndex.index);
                Serial.print("  F_key_num:");
                Serial.println(lDisplayIndex.F_key_num);

                break;

            case '2': //调节上键
                //**如果此时是修改状态
                if (lDisplayIndex.index_visiable)
                {
                    switch (lDisplayIndex.page)
                    {
                    case 1:
                        /* code */
                        break;

                    default:
                        break;
                    }
                }
                break;
            default:
                break;
            }
        }
        //*显示内容放入队列
        xStatus = xQueueGenericSend( ( xDisplayIndexQueue ), ( &lDisplayIndex ), ( ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 329 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                 0 
# 329 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
# 341 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                                                              0 
# 341 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                                              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));

            //再次尝试把数据放入队列
            xStatus = xQueueGenericSend( ( xDisplayIndexQueue ), ( &lDisplayIndex ), ( ( ( TickType_t ) ( ( ( uint32_t ) ( 30 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 344 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                     0 
# 344 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
# 358 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
              0 
# 358 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
              /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
}

/**

 * @brief 显示器显示任务

 * 

 * @param pvParameters 

 */
# 366 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
# 377 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                                                                   0 
# 377 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
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
                   
# 395 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 395 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                  "%02u/%02u %02u:%02u:%02u"
# 395 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  ); &__c[0];}))
# 395 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                                  ,
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
            // lcd.setCursor(Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].X,  //X位置
            //               Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].Y); //Y位置
            lcd.setCursor(8, //X位置
                          1); //Y位置

            char datestring2[10] = {};

            //将定时器数据内容组合为字符串
            snprintf_P(datestring2,
                       (sizeof(datestring2) / sizeof(datestring2[0])),
                       
# 432 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                      (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 432 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                      "%02u:%02u %02u"
# 432 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                      ); &__c[0];}))
# 432 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                                            ,
                       alarm[lDisplayIndex.page - 1].Hour(),
                       alarm[lDisplayIndex.page - 1].Minute(),
                       alarm[lDisplayIndex.page - 1].Second());

            lcd.print(datestring2); //显示数据
        }

        //*显示光标
        lcd.setCursor(Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].X, //X位置
                      Page[lDisplayIndex.page].UI_data[lDisplayIndex.index].Y); //Y位置

        lcd.cursor();

        Serial.print("page:");
        Serial.print(lDisplayIndex.page);
        Serial.print("  index:");
        Serial.println(lDisplayIndex.index);

        //退出临界区
        __asm__ __volatile__ ( "pop __tmp_reg__" "\n\t" "out __SREG__, __tmp_reg__" "\n\t" ::: "memory" );

        vTaskDelay(( ( TickType_t ) ( ( ( uint32_t ) ( 300 ) * ( TickType_t ) ( (TickType_t)( (uint32_t)128000 >> (
# 454 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino" 3
                  0 
# 454 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
                  /* portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick*/ + 11) ) ) /* 2^11 = 2048 WDT scaler for 128kHz Timer*/ ) / ( TickType_t ) 1000 ) ));
    }
}

/**

 * @brief 蜂鸣器启动函数

 * 

 */
# 462 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
void BUZZER_init()
{
    pinMode(22, 0x2);
    pinMode(26, 0x1);
    digitalWrite(26, 0x1);
}

/**

 * @brief 蜂鸣器响

 * 

 */
# 473 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
void BUZZER_SetHigh()
{
    digitalWrite(26, 0x0);
}

/**

 * @brief 蜂鸣器不响

 * 

 */
# 482 "d:\\01-Projects\\接的项目\\药盒程序\\程序\\Code\\Code.ino"
void BUZZER_SetLow()
{
    digitalWrite(26, 0x1);
}

/**

 * @brief 

 * 

 */
