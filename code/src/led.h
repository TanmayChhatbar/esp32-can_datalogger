#pragma once
#include "defs.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "sd_logger.h"

extern Adafruit_NeoPixel leds;

#define GRB_BRIGHTNESS 32
#define GRB_WHITE leds.Color(255 * GRB_BRIGHTNESS / 255, 255 * GRB_BRIGHTNESS / 255, 255 * GRB_BRIGHTNESS / 255)
#define GRB_BLACK leds.Color(0 , 0, 0)
#define GRB_RED leds.Color(255 * GRB_BRIGHTNESS / 255, 0, 0)
#define GRB_GREEN leds.Color(0, 255 * GRB_BRIGHTNESS / 255, 0)
#define GRB_BLUE leds.Color(0, 0, 255 * GRB_BRIGHTNESS / 255)
#define GRB_YELLOW leds.Color(255 * GRB_BRIGHTNESS / 255, 255 * GRB_BRIGHTNESS / 255, 0)
#define GRB_MAGENTA leds.Color(255 * GRB_BRIGHTNESS / 255, 0, 255 * GRB_BRIGHTNESS / 255)
#define GRB_CYAN leds.Color(0, 255 * GRB_BRIGHTNESS / 255, 255 * GRB_BRIGHTNESS / 255)

void setupLED();
bool handleLED(logging_mode_t logging_mode, unsigned long time_since_last_change);
void overrideLED(uint32_t color);
