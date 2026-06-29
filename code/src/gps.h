#pragma once
#include "pindef.h"
#include <Arduino.h>
#include <Adafruit_GPS.h>

#define GPSSerial Serial1

extern Adafruit_GPS GPS;

bool setupGPS();
bool waitForGPSFix(uint32_t timeout_ms = 60000);
bool readGPS();
void syncTimeFromGPS();
