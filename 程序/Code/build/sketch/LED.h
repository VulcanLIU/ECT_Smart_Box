#include "Arduino.h"

#define ALARM1_LED_P_PIN 20
#define ALARM1_LED_N_PIN 21
#define ALARM2_LED_P_PIN 18
#define ALARM2_LED_N_PIN 19
#define ALARM3_LED_P_PIN 16
#define ALARM3_LED_N_PIN 17
#define ALARM4_LED_P_PIN 14
#define ALARM4_LED_N_PIN 15

class LED
{
private:
    bool _state;
    uint8_t _P_pin;
    uint8_t _N_pin;

public:
    LED(uint8_t P_pin, uint8_t N_pin) : _P_pin(P_pin), _N_pin(N_pin), _state(LOW){};
    void init();
    void setHigh();
    void setLow();
    void update();
    bool state();
};

extern LED Alarm1_LED;
extern LED Alarm2_LED;
extern LED Alarm3_LED;
extern LED Alarm4_LED;
