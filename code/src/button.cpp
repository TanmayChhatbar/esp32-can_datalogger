#include "button.hpp"

bool Button::begin(unsigned int pin_number, unsigned long long_press_timer_ms, bool is_pulldown = false)
{
    _pin_number = pin_number;
    _long_press_timer_ms = long_press_timer_ms;
    _state = BUTTON_NOT_PRESSED;
    _is_pulldown = is_pulldown;
    return true;
}

button_state_t Button::readButtonState()
{
    unsigned long current_timer_ms = millis();
    bool current_read_state;
    if (_is_pulldown)
    {
        current_read_state = digitalRead(_pin_number); // HIGH is pressed, LOW is not pressed if !is_pulldown
    }
    else
    {
        current_read_state = !digitalRead(_pin_number); // HIGH is pressed, LOW is not pressed if !is_pulldown
    }

    if (current_read_state) // if currently pressed
    {
        if (_state == BUTTON_NOT_PRESSED) // if wasn't pressed before
        {
            _state = BUTTON_FRESH_PRESSED;
            _button_timer_ms = current_timer_ms;
        }
        else if (_state == BUTTON_LONG_PRESS_CONFIRMED || _state == BUTTON_LONG_PRESS_CONTINUED)
        {
            _state = BUTTON_LONG_PRESS_CONTINUED;
        }
        else if (current_timer_ms - _button_timer_ms > _long_press_timer_ms) // long press timer exceeded
        {
            _state = BUTTON_LONG_PRESS_CONFIRMED;
        }
        else
        {
            _state = BUTTON_PRESSED;
        }
    }
    else // button not currently pressed
    {
        if ((_state == BUTTON_PRESSED || _state == BUTTON_FRESH_PRESSED) && (current_timer_ms - _button_timer_ms < _long_press_timer_ms))
        {
            _state = BUTTON_SHORT_PRESS_CONFIRMED;
        }
        else
        {
            _state = BUTTON_NOT_PRESSED;
        }
    }
    return _state;
}

String buttonStateToString(button_state_t st)
{
    switch (st)
    {
    case BUTTON_NOT_PRESSED:
        return "BUTTON_NOT_PRESSED";
    case BUTTON_PRESSED:
        return "BUTTON_PRESSED";
    case BUTTON_FRESH_PRESSED:
        return "BUTTON_FRESH_PRESSED";
    case BUTTON_SHORT_PRESS_CONFIRMED:
        return "BUTTON_SHORT_PRESS_CONFIRMED";
    case BUTTON_LONG_PRESS_CONFIRMED:
        return "BUTTON_LONG_PRESS_CONFIRMED";
    case BUTTON_LONG_PRESS_CONTINUED:
        return "BUTTON_LONG_PRESS_CONTINUED";
    default:
        return "BUTTON_PRESS_UNKNOWN";
    }
}
