#include <Arduino.h>
#include "defs.h"

enum button_press_t
{
    BUTTON_FRESH_PRESSED,         // current state is pressed, last read state was not pressed
    BUTTON_PRESSED,               // current state is pressed
    BUTTON_SHORT_PRESS_CONFIRMED, // on depress, duration was less than long press threshold
    BUTTON_LONG_PRESS_CONFIRMED,  // duration since press is longer than threshold
    BUTTON_LONG_PRESS_CONTINUED,  // long press detected already
    BUTTON_NOT_PRESSED            // current state is not pressed, last read state was not pressed
};

typedef struct
{
    button_press_t state;
    int button_index;
} button_state_t;

class Button
{
public:
    bool begin(unsigned int pin_number, unsigned long long_press_timer_ms, int num_buttons, const float *voltages);
    button_state_t readButtonState();

private:
    unsigned int _pin_number;
    unsigned long _button_timer_ms;
    button_state_t _state = {.state = BUTTON_NOT_PRESSED, .button_index = -1};
    unsigned long long _long_press_timer_ms;
    bool _is_initialized = false;
    float *_voltages = nullptr;
    unsigned int _num_buttons = 0;
};

String buttonStateToString(button_state_t);
