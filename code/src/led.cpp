
#include "led.h"

Adafruit_NeoPixel leds(NUM_LED, ARGB_LED, NEO_GRB);

void setupLED()
{
    pinMode(ARGB_LED, OUTPUT);
    leds.begin();
    leds.setBrightness(255); // set brightness to maximum
    leds.setPixelColor(0, GRB_WHITE); // white
    leds.show();
}

void overrideLED(uint32_t color)
{
    leds.setPixelColor(0, color);
    leds.show();
}

bool handleLED(logging_mode_t logging_mode, unsigned long time_since_last_change)
{
    unsigned long toggle_time = 0;
    uint32_t new_color = GRB_BLACK;
    switch (logging_mode)
    {
        case LOGGING_MODE_SETUP:
            new_color = GRB_YELLOW;
            toggle_time = 50;
            break;
        case LOGGING_MODE_WAITGPS:
            new_color = GRB_MAGENTA;
            toggle_time = 1000;
            break;
        case LOGGING_MODE_INACTIVE:
            new_color = GRB_GREEN;
            toggle_time = -1;
            break;
        case LOGGING_MODE_STOP:
            new_color = GRB_RED;
            toggle_time = 50;
            break;
        case LOGGING_MODE_START:
            new_color = GRB_RED;
            toggle_time = 50;
            break;
        case LOGGING_MODE_ACTIVE:
            new_color = GRB_RED;
            toggle_time = 500;
            break;
        case LOGGING_MODE_ACTIVE_CAN:
            new_color = GRB_RED;
            toggle_time = 250;
            break;
        case LOGGING_MODE_WIFI_START:
            new_color = GRB_BLUE;
            toggle_time = 50;
            break;
        case LOGGING_MODE_WIFI_ACTIVE:
            new_color = GRB_BLUE;
            toggle_time = 500;
            break;
        case LOGGING_MODE_WIFI_STOP:
            new_color = GRB_BLUE;
            toggle_time = 50;
            break;
        default:
            new_color = GRB_WHITE;
            toggle_time = 200;
            break;
    }

    if (toggle_time == -1)
    {
        leds.setPixelColor(0, new_color);
        leds.show();
        return false;
    }

    // return whether LED state was toggled
    bool toggled = (time_since_last_change > toggle_time);
    if (toggled)
    {
        leds.setPixelColor(0, (leds.getPixelColor(0) == new_color) ? GRB_BLACK : new_color);
    }
    leds.show();
    return toggled;
}
