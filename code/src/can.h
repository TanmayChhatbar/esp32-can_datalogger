#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

TwaiCAN setupCAN(uint canSx, uint canTx, uint canRx);
void printMessageToSerialRaw(unsigned long long timestamp, CanFrame frame);
void printMessageToSerial(unsigned long long timestamp, CanFrame frame);
