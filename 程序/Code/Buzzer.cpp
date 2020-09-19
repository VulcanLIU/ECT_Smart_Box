#include "Buzzer.h"

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
