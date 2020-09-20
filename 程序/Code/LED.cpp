#include "LED.h"

uint8_t l_y = 2020;
uint8_t l_m = 10;
uint8_t l_d = 4;
uint8_t l_h = 12;
uint8_t l_mi = 0;
uint8_t l_s = 0;

void LED::init()
{
    pinMode(_P_pin, OUTPUT);
    digitalWrite(_P_pin, HIGH);
    pinMode(_N_pin, OUTPUT);
    digitalWrite(_N_pin, HIGH);
}

void LED::setHigh()
{
    digitalWrite(_N_pin, LOW);
    _state = HIGH;
}

void LED::setLow()
{
    digitalWrite(_N_pin, HIGH);
    _state = LOW;
}

void LED::update()
{
    if(_state)
    {
        setLow();
    }
    else
    {
        setHigh();
    }  
}

bool LED::state()
{
    return _state;
}

LED Alarm1_LED(ALARM1_LED_P_PIN, ALARM1_LED_N_PIN);
LED Alarm2_LED(ALARM2_LED_P_PIN, ALARM2_LED_N_PIN);
LED Alarm3_LED(ALARM3_LED_P_PIN, ALARM3_LED_N_PIN);
LED Alarm4_LED(ALARM4_LED_P_PIN, ALARM4_LED_N_PIN);
