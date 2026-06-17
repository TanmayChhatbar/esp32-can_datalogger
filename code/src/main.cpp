#include <Arduino.h>
#include <FastLED.h>
#include <ESP32-TWAI-CAN.hpp>
#include "pindef.h"
#include "led.h"
#include "can.h"
#include "sd.h"
#include "imu.h"
#include "gps.h"

TwaiCAN can0;
TwaiCAN can1;

void setup()
{
    Serial.begin(460800);

    // can0 = setupCAN(CAN0_SX, CAN0_TX, CAN0_RX);
    // can1 = setupCAN(CAN1_SX, CAN1_TX, CAN1_RX);
    setupLED();
}

void loop()
{
    static unsigned long long startTime = micros();
    static unsigned long long lastReceptionTime = micros();

    // CanFrame frame;
    // if (can0.readFrame(frame))
    // {
    //     unsigned long long timestamp = micros() - startTime;
    //     printMessageToSerialRaw(timestamp, frame);
    //     toggleLED();
    //     lastReceptionTime = micros();
    // }

    if (micros() - lastReceptionTime > 2000000)
    {
        Serial.println("Waiting for CAN frames...");
        lastReceptionTime = micros();
        toggleLED();
    }
}
