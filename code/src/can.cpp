#include "can.h"

bool setupCAN(TwaiCAN &can, uint canSx, uint canTx, uint canRx)
{
    pinMode(canSx, OUTPUT);
    digitalWrite(canSx, LOW); // HIGH for silent mode, (HIGH doesnt work for some reason, so just use LOW)
    can.setPins(canTx, canRx);
    can.setSpeed(can.convertSpeed(500));
    can.setTxQueueSize(10); // TODO check if needed
    can.setRxQueueSize(10);
    return can.begin();
}

void printMessageToSerialRaw(const unsigned long long timestamp, const CanFrame frame, const uint8_t bus)
{
    // TimeStamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8
    // 1781966822488000,0000020A,false,Rx,0,8,7A,8C,00,00,00,01,23,00,
    // Serial.printf("%.3f,", (float)(timestamp) / 1000000.0f); // TODO change to posix time in us
    // Serial.print(frame.identifier, HEX);
    // Serial.print(",");
    // Serial.print(frame.extd ? "1" : "0");
    // Serial.print(",");
    // Serial.print(frame.rtr ? "1" : "0");
    // Serial.print(",");
    // Serial.print(bus);
    // Serial.print(",");
    // Serial.print(frame.data_length_code);
    // Serial.print(",");
    // for (int i = 0; i < frame.data_length_code; i++)
    // {
    //     if (frame.data[i] < 0x10)
    //         Serial.print("0");
    //     Serial.print(frame.data[i], HEX);
    //     Serial.print(",");
    // }
    // Serial.println();
    Serial.println(CanFrameToString(frame, timestamp, bus));
}

String CanFrameToString(const CanFrame &frame, const unsigned long long timestamp, const uint8_t bus)
{
    String outString = String((float)timestamp / 1000000.0f, 3) + "," + 
                        String(frame.identifier, HEX) + "," + 
                        String(frame.extd ? "1" : "0") + "," + 
                        String(frame.rtr ? "1" : "0") + "," + 
                        String(frame.data_length_code) + ",";
    for (int i = 0; i < frame.data_length_code; i++)
    {
        if (frame.data[i] < 0x10)
            outString += "0";
        outString += String(frame.data[i], HEX) + ",";
    }
    return outString;
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
