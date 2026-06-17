
#include "led.h"

CRGB leds[NUM_LED];

void setupLED()
{
    pinMode(ARGB_LED, OUTPUT);
    FastLED.addLeds<NEOPIXEL, ARGB_LED>(leds, NUM_LED);
    FastLED.setBrightness(8);
}

void toggleLED()
{
    leds[0] = leds[0] == CRGB::Black ? CRGB::Green : 
              leds[0] == CRGB::Green ? CRGB::Blue : 
              leds[0] == CRGB::Blue ? CRGB::Red :
              leds[0] == CRGB::Red ? CRGB::Yellow :
              leds[0] == CRGB::Yellow ? CRGB::Cyan :
              leds[0] == CRGB::Cyan ? CRGB::Magenta :
              leds[0] == CRGB::Magenta ? CRGB::White : 
              CRGB::Black;
    
    for (uint i = 1; i < NUM_LED; i++)
    {
        leds[i] = leds[0];
    }
    FastLED.show();
}
