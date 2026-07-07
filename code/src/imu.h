#pragma once
#include "pindef.h"
#include "can.h"
#include "canmod_gps.h"

bool setupIMU();
bool readIMU();
bool IMUFrameToCanFrame(log_frame_t log_frame);
