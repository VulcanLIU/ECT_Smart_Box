#include <Arduino.h>

//定义函数指针类型
typedef void (*voidFuncPtr)(void);

//
void Photogate_init();

//四个光电门上升沿中断
void Photogate1_ISR();
void Photogate2_ISR();
void Photogate3_ISR();
void Photogate4_ISR();

//剩余需要通过药量
extern int8_t Pill_left[4];

//中断引脚
extern uint8_t Photogate_PIN_ARRY[4];
//中断服务函数数组
extern voidFuncPtr Photogate_ISR[4];
