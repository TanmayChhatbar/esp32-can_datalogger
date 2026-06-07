#include <Arduino.h>
#include <ESP32-TWAI-CAN.hpp>

#define CAN0_TX 10    // wire to CTX
#define CAN0_RX 11    // wire to CRX
#define CAN0_SX 12    // wire to CSX

#define CAN1_TX 13    // wire to CTX
#define CAN1_RX 14    // wire to CRX
#define CAN1_SX 15    // wire to CSX

TwaiCAN setupCAN(uint canSx, uint canTx, uint canRx);
void printMessageToSerialRaw(unsigned long long timestamp, CanFrame frame);
void printMessageToSerial(unsigned long long timestamp, CanFrame frame);
