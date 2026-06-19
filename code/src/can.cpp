#include "can.h"

bool setupCAN(TwaiCAN &can, uint canSx, uint canTx, uint canRx)
{
    pinMode(canSx, OUTPUT);
    digitalWrite(canSx, LOW); // HIGH for silent mode, (HIGH doesnt work for some reason, so just use LOW)
    can.setPins(canTx, canRx);
    can.setSpeed(can.convertSpeed(500));
    can.setTxQueueSize(10); // TODO check if needed
    can.setRxQueueSize(10);
    if (!can.begin())
    {
        Serial.println("Failed to initialize CAN");
        return false;
    }
    Serial.println("CAN initialized successfully");
    return true;
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
    // TODO
}

bool requestOBDSignal(TwaiCAN &can, OBD_signal signal)
{
    CanFrame frame;
    frame.identifier = 0x7DF; // functional request ID for OBD2
    frame.data_length_code = 8;
    frame.data[0] = 0x02;   // number of additional bytes (Service ID + PID)
    frame.data[1] = 0x01;   // service ID for current data
    frame.data[2] = signal; // PID for the requested signal
    for (int i = 3; i < 8; i++)
    {
        frame.data[i] = 0x00; // padding
    }
    return can.writeFrame(frame, 0);
}
