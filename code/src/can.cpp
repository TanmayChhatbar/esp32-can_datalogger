#include "can.h"

void setupCAN()
{
    pinMode(CAN_SX, OUTPUT);
    digitalWrite(CAN_SX, LOW); // HIGH for silent mode, (HIGH doesnt work for some reason, so just use LOW)
    ESP32Can.setPins(CAN_TX, CAN_RX);
    ESP32Can.setSpeed(ESP32Can.convertSpeed(500));
    if (!ESP32Can.begin())
    {
        Serial.println("Failed to initialize CAN");
        while (1)
            ;
    }
    Serial.println("CAN initialized successfully");
}

void printMessageToSerialRaw(unsigned long long timestamp, CanFrame frame)
{

    Serial.printf("%.3f\t", (float)(timestamp) / 1000000.0f);
    Serial.print(frame.identifier, HEX);
    Serial.print("\t");
    for (int i = 0; i < frame.data_length_code; i++)
    {
        if (frame.data[i] < 0x10)
            Serial.print("0");
        Serial.print(frame.data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void printMessageToSerial(unsigned long long timestamp, CanFrame frame)
{
    //TODO
}
