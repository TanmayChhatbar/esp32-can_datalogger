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
volatile uint OBD_sigCount_1Hz_index = 0;
volatile uint OBD_sigCount_10Hz_index = 0;
volatile OBD_signal OBD_1Hz_ID = OBD_SIGNALS_OF_INTEREST_1HZ[0];
volatile OBD_signal OBD_10Hz_ID = OBD_SIGNALS_OF_INTEREST_10HZ[0];
volatile unsigned long long lastOBDRequestTime_1Hz = 0;
volatile unsigned long long lastOBDRequestTime_10Hz = 0;

void vTask_OBD_Request_1Hz(void *parameter)
{
    const TickType_t period = pdMS_TO_TICKS(1000);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;)
    {
        OBD_signal target = OBD_SIGNALS_OF_INTEREST_1HZ[OBD_sigCount_1Hz_index];
        if (target != OBD_1Hz_ID || micros() - lastOBDRequestTime_1Hz > 200000)
        {
            OBD_1Hz_ID = target;

            if (!requestOBDSignal(can0, OBD_1Hz_ID))
            {
                Serial.println("Failed to send OBD request (1Hz)");
            }
            else
            {
                lastOBDRequestTime_1Hz = micros();
            }
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

void setup()
{
    Serial.begin(460800);

    // set up on-board LEDs
    setupLED();

    // set up GPS
    if (!setupGPS())
        Serial.println("Failed to set up GPS");

    // set up IMU
    if (!setupIMU())
        Serial.println("Failed to set up IMU");

    // set up SD card
    if (!setupSD())
        Serial.println("Failed to set up SD card");

    // set up CAN buses
    TwaiCAN can0;
    if (!setupCAN(can0, CAN0_SX, CAN0_TX, CAN0_RX))
        Serial.println("Failed to set up CAN bus 0");

#if TWO_CAN
    TwaiCAN can1;
    if (!setupCAN(can1, CAN1_SX, CAN1_TX, CAN1_RX))
        Serial.println("Failed to set up CAN bus 1");
#endif

    // init tasks
    xTaskCreatePinnedToCore(vTask_OBD_Request_1Hz, "OBD_Request_1Hz", 4096, NULL, 1, NULL, 0);
}

void loop()
{
    static unsigned long long startTime = micros();
    static unsigned long long lastReceptionTime = startTime;
    static unsigned long long lastRequestTime_1Hz = startTime;
    static unsigned long long lastRequestTime_10Hz = startTime;
    static unsigned long long currentTime = startTime;
    currentTime = micros();

    CanFrame frame;
    if (can0.readFrame(frame) && (frame.identifier >= 0x7E8 && frame.identifier <= 0x7EF))
    {
        unsigned long long timestamp = currentTime - startTime;
        switch (frame.data[2])
        {
        case OBD_ENGINE_RPM:
        {
            Serial.print(timestamp);
            float engine_rpm = (frame.data[3] * 256 + frame.data[4]) / 4.0;
            Serial.println("\tEngine Speed:\t" + String(engine_rpm) + "rpm");
            break;
        }

        case OBD_VEHICLE_SPEED:
        {
            Serial.print(timestamp);
            float vehicle_speed = frame.data[3];
            Serial.println("\tVehicle Speed:\t" + String(vehicle_speed) + "km/h");
            break;
        }
        case OBD_GEAR_POSITION:
        {
            Serial.print(timestamp);
            printMessageToSerialRaw(timestamp, frame);
            // float gear_pos = (frame.data[3] * 256 + frame.data[4]) / 100.0;
            // Serial.println("\tGear:\t" + String(gear_pos))
        }
        }
        lastReceptionTime = currentTime;

        uint8_t mode = frame.data[1]; // should be 0x41 for response to 01
        uint8_t pid = frame.data[2];
        if (mode == 0x41 && pid == OBD_1Hz_ID)
        {
            OBD_sigCount_1Hz_index = (OBD_sigCount_1Hz_index + 1) % OBD_SIGCOUNT_1HZ; // cycle through requested OBD signals
        }
    }

    // request OBD signals
    if (currentTime - lastRequestTime_1Hz > 100000)
    {
        bool success = true;
        // TODO only last request is replied to
        // for (OBD_signal signal : {OBD_SIGNALS_OF_INTEREST_10HZ})
        for (OBD_signal signal : {OBD_GEAR_POSITION})
        {
            success = success && requestOBDSignal(can0, signal);
        }
        if (!success)
        {
            Serial.print(currentTime - startTime);
            Serial.println("\tFailed to send OBD request (10Hz)");
        }
        lastRequestTime_1Hz = currentTime;
    }
    // if (currentTime - lastRequestTime_1Hz > 1000000)
    // {
    //     bool success = true;
    //     for (OBD_signal signal : OBD_SIGNALS_OF_INTEREST_1HZ)
    //     {
    //         success = success && requestOBDSignal(can0, signal);
    //     }
    //     if (!success)
    //     {
    //         Serial.print(currentTime - startTime);
    //         Serial.println("\tFailed to send OBD request (10Hz)");
    //     }
    //     lastRequestTime_1Hz = currentTime;
    // }

    // print CAN timeout if no frames received
    if (currentTime - lastReceptionTime > 2000000)
    {
        Serial.println("Waiting for OBD response...");
        lastReceptionTime = currentTime;
        toggleLED();
    }
}
