#pragma once
#include "defs.h"
#include <Arduino.h>
#include <FastLED.h>
#include "sd_logger.h"

void setupLED();
bool handleLED(logging_mode_t logging_mode, unsigned long time_since_last_change);
