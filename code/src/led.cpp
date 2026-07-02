
#include "led.h"

CRGB leds[NUM_LED];

void setupLED()
{
    pinMode(ARGB_LED, OUTPUT);
    FastLED.addLeds<WS2812B, ARGB_LED, GRB>(leds, NUM_LED);
    FastLED.setBrightness(8);
    leds[0] = CRGB::White;
    FastLED.show();
}


bool handleLED(logging_mode_t logging_mode, unsigned long time_since_last_change)
{
    unsigned long toggle_time = 0;
    CRGB new_color = CRGB::Black;
    switch (logging_mode)
    {
        case LOGGING_MODE_WAITGPS:
            new_color = CRGB::Magenta;
            toggle_time = 1e8;
            break;
        case LOGGING_MODE_INACTIVE:
            new_color = CRGB::Yellow;
            toggle_time = 500;
            break;
        case LOGGING_MODE_STOP:
            new_color = CRGB::Yellow;
            toggle_time = 1e8;
            break;
        case LOGGING_MODE_START:
            new_color = CRGB::Red;
            toggle_time = 1e8;
            break;
        case LOGGING_MODE_ACTIVE:
            new_color = CRGB::Red;
            toggle_time = 500;
            break;
        case LOGGING_MODE_WIFI:
            new_color = CRGB::Blue;
            toggle_time = 500;
            break;
        default:
            new_color = CRGB::White;
            toggle_time = 200;
            break;
    }

    // return if LED state was toggled
    bool toggled = (time_since_last_change > toggle_time);
    if (toggled)
    {
        leds[0] = (leds[0] == new_color) ? CRGB::Black : new_color;
    }
    FastLED.show();
    return toggled;
}
