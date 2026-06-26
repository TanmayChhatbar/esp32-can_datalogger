#pragma once
#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

enum OBD_signal : uint8_t
{
    OBD_ENGINE_LOAD = 0x04,
    OBD_COOLANT_TEMP = 0x05,
    OBD_INTAKE_AIR_TEMP = 0x06,
    OBD_MAF_FLOW_RATE = 0x07,
    OBD_ENGINE_RPM = 0x0C,
    OBD_VEHICLE_SPEED = 0x0D,
    OBD_THROTTLE_POSITION = 0x11,
    OBD_RUNNING_TIME = 0x1F,
    OBD_FUEL_LEVEL = 0x2F,
    OBD_ODOMETER = 0xA6
};

const OBD_signal OBD_SIGNALS_OF_INTEREST_1HZ[] = {OBD_FUEL_LEVEL, OBD_RUNNING_TIME, OBD_ODOMETER};
const OBD_signal OBD_SIGNALS_OF_INTEREST_10HZ[] = {OBD_ENGINE_LOAD, OBD_VEHICLE_SPEED};

bool setupCAN(TwaiCAN &can, uint canSx, uint canTx, uint canRx);
void printMessageToSerialRaw(const unsigned long long timestamp, const CanFrame frame, const uint8_t bus);
bool requestOBDSignal(TwaiCAN &can, OBD_signal signal);
String CanFrameToString(const CanFrame &frame, const unsigned long long timestamp, const uint8_t bus);
const uint8_t OBD_SIGCOUNT_1HZ = sizeof(OBD_SIGNALS_OF_INTEREST_1HZ) / sizeof(OBD_signal);
const uint8_t OBD_SIGCOUNT_10HZ = sizeof(OBD_SIGNALS_OF_INTEREST_10HZ) / sizeof(OBD_signal);
