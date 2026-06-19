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
#if TWO_CAN
TwaiCAN can1;
#endif

void setup()
{
    Serial.begin(460800);

    // set up on-board LEDs
    setupLED();

    // set up GPS
    if (!setupGPS())
    {
        Serial.println("Failed to set up GPS");
    }

    // set up IMU
    if (!setupIMU())
    {
        Serial.println("Failed to set up IMU");
    }

    // set up SD card
    if (!setupSD())
    {
        Serial.println("Failed to set up SD card");
    }

    // set up CAN buses
    TwaiCAN can0;
    if (!setupCAN(can0, CAN0_SX, CAN0_TX, CAN0_RX))
    {
        Serial.println("Failed to set up CAN bus 0");
    }
#if TWO_CAN
    TwaiCAN can1;
    if (!setupCAN(can1, CAN1_SX, CAN1_TX, CAN1_RX))
    {
        Serial.println("Failed to set up CAN bus 1");
    }
#endif
}

void loop()
{
    static unsigned long long startTime = micros();
    static unsigned long long lastReceptionTime = startTime;
    static unsigned long long lastRequestTime_1Hz = startTime;
    static unsigned long long lastRequestTime_10Hz = startTime;
    static unsigned long long currentTime = startTime;
    currentTime = micros();

    // read CAN data
    CanFrame frame;
    if (can0.readFrame(frame))
    {
        unsigned long long timestamp = currentTime - startTime;
        if (frame.identifier != 0x7E8)
        {
            printMessageToSerialRaw(timestamp, frame);
            toggleLED();
        }
        lastReceptionTime = currentTime;
    }

    // request OBD signals
    if (currentTime - lastRequestTime_1Hz > 1000000)
    {
        bool success = true;
        for (OBD_signal signal : OBD_SIGNALS_OF_INTEREST_1HZ)
        {
            success = success && requestOBDSignal(can0, signal);
        }
        if (!success)
        {
            Serial.print(currentTime - startTime);
            Serial.println("\tFailed to send OBD request (1Hz)");
        }
        lastRequestTime_1Hz = currentTime;
    }
    if (currentTime - lastRequestTime_10Hz > 100000)
    {
        bool success = true;
        for (OBD_signal signal : OBD_SIGNALS_OF_INTEREST_10HZ)
        {
            success = success && requestOBDSignal(can0, signal);
        }
        if (!success)
        {
            Serial.print(currentTime - startTime);
            Serial.println("\tFailed to send OBD request (10Hz)");
        }
        lastRequestTime_10Hz = currentTime;
    }

    // print CAN timeout if no frames received
    if (currentTime - lastReceptionTime > 2000000)
    {
        Serial.println("Waiting for CAN frames...");
        lastReceptionTime = currentTime;
        toggleLED();
    }
}
