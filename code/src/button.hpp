#include <Arduino.h>

enum button_state_t
{
    BUTTON_FRESH_PRESSED,         // current state is pressed, last read state was not pressed
    BUTTON_PRESSED,               // current state is pressed
    BUTTON_SHORT_PRESS_CONFIRMED, // on depress, duration was less than long press threshold
    BUTTON_LONG_PRESS_CONFIRMED,  // duration since press is longer than threshold
    BUTTON_LONG_PRESS_CONTINUED,  // long press detected already
    BUTTON_NOT_PRESSED            // current state is not pressed, last read state was not pressed
};

class Button
{
public:
    bool begin(unsigned int pin_number, unsigned long long_press_timer_ms, bool is_pulldown);
    button_state_t readButtonState();
private:
    unsigned int _pin_number;
    unsigned long _button_timer_ms;
    button_state_t _state = BUTTON_NOT_PRESSED;
    unsigned long long _long_press_timer_ms;
    bool _is_pulldown;
    bool _is_initialized = false;
};

String buttonStateToString(button_state_t);
