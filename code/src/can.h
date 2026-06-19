#pragma once
#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

bool setupCAN(TwaiCAN &can, uint canSx, uint canTx, uint canRx);
void printMessageToSerialRaw(unsigned long long timestamp, CanFrame frame);
void printMessageToSerial(unsigned long long timestamp, CanFrame frame);

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
    OBD_ODOMETER = 0xA6,
    OBD_GEAR_POSITION = 0xA4
};

#define OBD_SIGNALS_OF_INTEREST_1HZ {OBD_FUEL_LEVEL}
#define OBD_SIGNALS_OF_INTEREST_10HZ {OBD_ENGINE_LOAD, OBD_COOLANT_TEMP, OBD_GEAR_POSITION, OBD_INTAKE_AIR_TEMP, OBD_MAF_FLOW_RATE}

bool requestOBDSignal(TwaiCAN &can, OBD_signal signal);
