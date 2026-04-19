#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

#define CAN_TX 10    // wire to CTX
#define CAN_RX 11    // wire to CRX
#define CAN_SX 12    // wire to CSX

void setupCAN();
void printMessageToSerialRaw(unsigned long long timestamp, CanFrame frame);
void printMessageToSerial(unsigned long long timestamp, CanFrame frame);
