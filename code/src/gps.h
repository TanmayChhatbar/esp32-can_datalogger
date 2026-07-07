#pragma once
#include "pindef.h"
#include <Arduino.h>
#include <Adafruit_GPS.h>
#include "can.h"
#include "canmod_gps.h"

#define GPSSerial Serial1

extern Adafruit_GPS GPS;

bool setupGPS();
bool readGPS();
void syncTimeFromGPS();
bool GPSFrameToCanFrame(log_frame_t log_frame[4]);
