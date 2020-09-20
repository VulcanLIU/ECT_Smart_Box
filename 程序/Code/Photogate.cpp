#include "Photogate.h"

//
int8_t Pill_left[4] = {0, 0, 0, 0};

//
uint8_t Photogate_PIN_ARRY[4] = {18, 19, 20, 21};

//
voidFuncPtr Photogate_ISR[4] = {Photogate1_ISR,
                                Photogate2_ISR,
                                Photogate3_ISR,
                                Photogate4_ISR};


//
long _millis[4] = {0, 0, 0, 0};

void Photogate_init()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(Photogate_PIN_ARRY[i], INPUT_PULLUP);
    }
}
//四个光电门上升沿中断
void Photogate1_ISR()
{
    uint8_t index = 0;

    if (millis() - _millis[index] > 100 && digitalRead(Photogate_PIN_ARRY[index]) == HIGH)
    {
        _millis[index] = millis();

        if (Pill_left[index]-- < 0)
        {
            Pill_left[index] = 0;
        }
    }
}

void Photogate2_ISR()
{
    uint8_t index = 1;

    if (millis() - _millis[index] > 100 && digitalRead(Photogate_PIN_ARRY[index]) == HIGH)
    {
        _millis[index] = millis();

        if (Pill_left[index]-- < 0)
        {
            Pill_left[index] = 0;
        }
    }
}

void Photogate3_ISR()
{
    uint8_t index = 2;

    if (millis() - _millis[index] > 100 && digitalRead(Photogate_PIN_ARRY[index]) == HIGH)
    {
        _millis[index] = millis();

        if (Pill_left[index]-- < 0)
        {
            Pill_left[index] = 0;
        }
    }
}

void Photogate4_ISR()
{
    uint8_t index = 3;

    if (millis() - _millis[index] > 100 && digitalRead(Photogate_PIN_ARRY[index]) == HIGH)
    {
        _millis[index] = millis();

        if (Pill_left[index]-- < 0)
        {
            Pill_left[index] = 0;
        }
    }
}
