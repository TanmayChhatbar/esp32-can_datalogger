#include "button.hpp"

bool Button::begin(unsigned int pin_number,
                   unsigned long long_press_timer_ms,
                   int num_buttons,
                   const float *voltages)
{
    _pin_number = pin_number;
    _long_press_timer_ms = long_press_timer_ms;
    _state = {.state = BUTTON_NOT_PRESSED, .button_index = -1};
    _num_buttons = num_buttons;

    _voltages = new float[num_buttons];
    for (int i = 0; i < num_buttons; i++)
    {
        _voltages[i] = voltages[i];
    }
    _is_initialized = true;

    return true;
}

button_state_t Button::readButtonState()
{
    if (!_is_initialized)
    {
        _state.state = BUTTON_NOT_PRESSED;
        _state.button_index = -1;
        return _state;
    }
    unsigned long current_timer_ms = millis();
    float current_voltage = analogRead(_pin_number) * (3.3f / 4095.0f);

    // find voltage close in _voltages array
    int button_index_tmp = -1;
    for (unsigned int i = 0; i < _num_buttons; i++)
    {
        if (fabs(current_voltage - _voltages[i]) < 0.1f)
        {
            button_index_tmp = i;
            break;
        }
    }

    if (button_index_tmp != -1) // if currently pressed
    {
        if (button_index_tmp != _state.button_index) // if button index changed, assume fresh press
        {
            _state.state = BUTTON_FRESH_PRESSED;
            _state.button_index = button_index_tmp;
            _button_timer_ms = current_timer_ms;
        }
        else if (_state.state == BUTTON_NOT_PRESSED) // if wasn't pressed before
        {
            _state.state = BUTTON_FRESH_PRESSED;
            _button_timer_ms = current_timer_ms;
        }
        else if (_state.state == BUTTON_LONG_PRESS_CONFIRMED || _state.state == BUTTON_LONG_PRESS_CONTINUED)
        {
            _state.state = BUTTON_LONG_PRESS_CONTINUED;
        }
        else if (current_timer_ms - _button_timer_ms > _long_press_timer_ms) // long press timer exceeded
        {
            _state.state = BUTTON_LONG_PRESS_CONFIRMED;
        }
        else
        {
            _state.state = BUTTON_PRESSED;
        }
    }
    else // button not currently pressed
    {
        if ((_state.state == BUTTON_PRESSED || _state.state == BUTTON_FRESH_PRESSED) && (current_timer_ms - _button_timer_ms < _long_press_timer_ms))
        {
            _state.state = BUTTON_SHORT_PRESS_CONFIRMED;
            // _state.button_index = (keep as it was before, to pass the index of the button that was just released)
        }
        else
        {
            _state.state = BUTTON_NOT_PRESSED;
            _state.button_index = -1;
        }
    }
    return _state;
}

String buttonStateToString(button_state_t st)
{
    String result = "(" + String(st.button_index) + ") ";
    switch (st.state)
    {
    case BUTTON_NOT_PRESSED:
        result += "BUTTON_NOT_PRESSED";
        break;
    case BUTTON_PRESSED:
        result += "BUTTON_PRESSED";
        break;
    case BUTTON_FRESH_PRESSED:
        result += "BUTTON_FRESH_PRESSED";
        break;
    case BUTTON_SHORT_PRESS_CONFIRMED:
        result += "BUTTON_SHORT_PRESS_CONFIRMED";
        break;
    case BUTTON_LONG_PRESS_CONFIRMED:
        result += "BUTTON_LONG_PRESS_CONFIRMED";
        break;
    case BUTTON_LONG_PRESS_CONTINUED:
        result += "BUTTON_LONG_PRESS_CONTINUED";
        break;
    default:
        result += "BUTTON_PRESS_UNKNOWN";
        break;
    }
    return result;
}
