
#include "led.h"

CRGB leds[1];

void setupLED()
{
    pinMode(LED_BUILTIN, OUTPUT);
    FastLED.addLeds<NEOPIXEL, LED_BUILTIN>(leds, 1);
    FastLED.setBrightness(8);
}

void toggleLED()
{
    leds[0] = leds[0] == CRGB::Black ? CRGB::Green : CRGB::Black;
    FastLED.show();
}
