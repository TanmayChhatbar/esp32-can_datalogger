#pragma once
#include "defs.h"
#include "can.h"
#include "can_imu_14bit.h"
#include "canmod_gps.h"
#include <SPI.h>
#include <SD.h>

enum logging_mode_t
{
    LOGGING_MODE_SETUP = -2,
    LOGGING_MODE_WAITGPS = -1,
    LOGGING_MODE_INACTIVE = 0,
    LOGGING_MODE_STOP = 1,
    LOGGING_MODE_START = 2,
    LOGGING_MODE_ACTIVE = 3,
    LOGGING_MODE_ACTIVE_CAN = 4,
    LOGGING_MODE_WIFI_START = 5,
    LOGGING_MODE_WIFI_ACTIVE = 6,
    LOGGING_MODE_WIFI_STOP = 7,
    LOGGING_MODE_TEST = 8
};

bool writeCanFrameToSD(const log_frame_t &log_frame, File &file);
bool writeCANHeaderToSD(File &file);
const char *getFilenameForCurrentTime();
uint64_t getEpochMicroseconds();
String loggingModeToString(logging_mode_t logging_mode);
uint64_t getEpochMicroseconds();
