#pragma once
#include "pindef.h"
#include "can.h"

bool setupSD();
bool writeCanFrameToSD(const CanFrame &frame, unsigned long long timestamp, const uint8_t bus);
