#include "can.h"

TwaiCAN setupCAN(uint canSx, uint canTx, uint canRx)
{
    pinMode(canSx, OUTPUT);
    digitalWrite(canSx, LOW); // HIGH for silent mode, (HIGH doesnt work for some reason, so just use LOW)
    TwaiCAN can;
    can.setPins(canTx, canRx);
    can.setSpeed(can.convertSpeed(500));
    if (!can.begin())
    {
        Serial.println("Failed to initialize CAN");
        while (1)
            ;
    }
    Serial.println("CAN initialized successfully");
    return can;
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
