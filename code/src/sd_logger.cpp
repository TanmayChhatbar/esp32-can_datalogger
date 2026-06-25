#include "sd_logger.h"

bool writeCanFrameToSD(const CanFrame &frame, unsigned long long timestamp, const uint8_t bus)
{
    Serial.println("SD TODO " + CanFrameToString(frame, timestamp, bus));
    return true;
}

File createNewLogFile(const String filename)
{
    Serial.println("SD TODO createNewLogFile: " + filename);
    return File();
}

bool writeToFile(File &file, const String &data)
{
    Serial.println("SD TODO writeToFile: " + data);
    return true;
}
