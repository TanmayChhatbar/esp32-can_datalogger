#pragma once
#include "pindef.h"
#include "can.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>

bool writeCanFrameToSD(const CanFrame &frame, unsigned long long timestamp, const uint8_t bus);
File createNewLogFile(const String filename);
bool writeToFile(File &file, const String &data);
