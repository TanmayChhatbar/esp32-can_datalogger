#include "sd.h"

bool setupSD()
{
    Serial.println("SD TODO");
    return false;
}

bool writeCanFrameToSD(const CanFrame &frame, unsigned long long timestamp, const uint8_t bus)
{
    Serial.println("SD TODO " + CanFrameToString(frame, timestamp, bus));
    return true;
}
