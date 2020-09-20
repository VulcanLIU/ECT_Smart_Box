#include "Arduino.h"

#define ALARM1_LED_P_PIN A14
#define ALARM1_LED_N_PIN A15
#define ALARM2_LED_P_PIN A12
#define ALARM2_LED_N_PIN A13
#define ALARM3_LED_P_PIN A10
#define ALARM3_LED_N_PIN A11
#define ALARM4_LED_P_PIN A8
#define ALARM4_LED_N_PIN A9

extern uint8_t l_y;
extern uint8_t l_m;
extern uint8_t l_d;
extern uint8_t l_h;
extern uint8_t l_mi;
extern uint8_t l_s;

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
