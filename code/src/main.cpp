#include <Arduino.h>
#include <FastLED.h>
#include <ESP32-TWAI-CAN.hpp>
#include "led.h"
#include "can.h"

void setup()
{
    Serial.begin(460800);

    setupCAN();
    setupLED();
}

void loop()
{
    static unsigned long long startTime = micros();
    static unsigned long long lastReceptionTime = micros();

    CanFrame frame;
    if (ESP32Can.readFrame(frame))
    {
        unsigned long long timestamp = micros() - startTime;
        printMessageToSerialRaw(timestamp, frame);
        toggleLED();
        lastReceptionTime = micros();
    }
    if (micros() - lastReceptionTime > 2000000)
    {
        Serial.println("Waiting for CAN frames...");
        lastReceptionTime = micros();
        toggleLED();
    }
}
