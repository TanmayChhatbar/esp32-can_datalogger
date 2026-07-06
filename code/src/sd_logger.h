#pragma once
#include "pindef.h"
#include "can.h"
#include <SPI.h>
#include <SD.h>

enum logging_mode_t
{
    LOGGING_MODE_WAITGPS = -1,
    LOGGING_MODE_INACTIVE = 0,
    LOGGING_MODE_STOP = 1,
    LOGGING_MODE_START = 2,
    LOGGING_MODE_ACTIVE = 3,
    LOGGING_MODE_WIFI_START = 4,
    LOGGING_MODE_WIFI_ACTIVE = 5,
    LOGGING_MODE_WIFI_STOP = 6
};

bool writeCanFrameToSD(const twai_message_t &frame, unsigned long long timestamp, const uint8_t bus, File &file);
bool writeCANHeaderToSD(File &file);
const char *getFilenameForCurrentTime();
uint64_t getEpochMicroseconds();
String loggingModeToString(logging_mode_t logging_mode);
uint64_t getEpochMicroseconds();
