#include <Arduino.h>
#include <FastLED.h>
#include <ESP32-TWAI-CAN.hpp>
#include "pindef.h"
#include "led.h"
#include "can.h"
#include "sd_logger.h"
#include "imu.h"
#include "gps.h"

TwaiCAN can0;
TwaiCAN can1;
volatile int OBD_sigCount_1Hz_index = 0;
volatile int OBD_sigCount_10Hz_index = 0;
volatile OBD_signal OBD_1Hz_ID = OBD_SIGNALS_OF_INTEREST_1HZ[0];   // current OBD signal for which response is expected, -1 means no signal requested yet for delay between request batches
volatile OBD_signal OBD_10Hz_ID = OBD_SIGNALS_OF_INTEREST_10HZ[0]; // current OBD signal for which response is expected, -1 means no signal requested yet for delay between request batches
volatile unsigned long long lastOBDRequestTime_1Hz_us = 0;
volatile unsigned long long lastOBDRequestTime_10Hz_us = 0;

void vTask_OBD_Request_1Hz(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    for (;;)
    {
        // if starting fresh, wait 1Hz equivalent time
        if (OBD_sigCount_1Hz_index == -1)
        {
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000)); // repeat every 1s
            OBD_sigCount_1Hz_index = 0;
            continue;
        }

        OBD_signal target = OBD_SIGNALS_OF_INTEREST_1HZ[OBD_sigCount_1Hz_index];
        // if target changed OR 50 ms timeout for OBD response, send request for next target
        if (target != OBD_1Hz_ID || micros() - lastOBDRequestTime_1Hz_us > 50000)
        {
            OBD_1Hz_ID = target;

            if (!requestOBDSignal(can0, OBD_1Hz_ID))
            {
                Serial.println("(vTask_OBD_Request_1Hz)\tOBD request failed (1Hz): " + String(OBD_1Hz_ID));
            }
            else
            {
                lastOBDRequestTime_1Hz_us = micros();
                Serial.println("(vTask_OBD_Request_1Hz)\tRequested OBD signal (1Hz): " + String(OBD_1Hz_ID, HEX));
            }
        }
    }
}

void vTask_OBD_Request_10Hz(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    for (;;)
    {
        // if starting fresh, wait 1Hz equivalent time
        if (OBD_sigCount_10Hz_index == -1)
        {
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000)); // repeat every 1s
            OBD_sigCount_10Hz_index = 0;
            continue;
        }

        OBD_signal target = OBD_SIGNALS_OF_INTEREST_10HZ[OBD_sigCount_10Hz_index];
        // if target changed OR 50 ms timeout for OBD response, send request for next target
        if (target != OBD_10Hz_ID || micros() - lastOBDRequestTime_10Hz_us > 50000)
        {
            OBD_10Hz_ID = target;

            if (!requestOBDSignal(can0, OBD_10Hz_ID))
            {
                Serial.println("(vTask_OBD_Request_10Hz)\tOBD request failed (10Hz): " + String(OBD_10Hz_ID));
            }
            else
            {
                lastOBDRequestTime_10Hz_us = micros();
                Serial.println("(vTask_OBD_Request_10Hz)\tRequested OBD signal (10Hz): " + String(OBD_10Hz_ID, HEX));
            }
        }
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
    if (!SD.begin(SPI_CS_SD))
        Serial.println("Failed to set up SD card");

    // set up CAN buses
    TwaiCAN can0;
    if (!setupCAN(can0, CAN0_SX, CAN0_TX, CAN0_RX))
        Serial.println("Failed to set up CAN bus 0");

    TwaiCAN can1;
    if (!setupCAN(can1, CAN1_SX, CAN1_TX, CAN1_RX))
        Serial.println("Failed to set up CAN bus 1");

    // init tasks
    // xTaskCreatePinnedToCore(vTask_OBD_Request_1Hz, "OBD_Request_1Hz", 4096, NULL, 1, NULL, 0);
    // xTaskCreatePinnedToCore(vTask_OBD_Request_10Hz, "OBD_Request_10Hz", 4096, NULL, 1, NULL, 0);
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
    if (can0.readFrame(frame))
    {
        // log timestamps
        unsigned long long timestamp = currentTime - startTime;
        lastReceptionTime = currentTime;

        // OBD response handling
        if (frame.identifier >= 0x7E8 && frame.identifier <= 0x7EF)
        {
            uint8_t mode = frame.data[1]; // should be 0x41 for response to 01
            uint8_t pid = frame.data[2];
            if (mode == 0x41 && pid == OBD_1Hz_ID)
            {
                OBD_sigCount_1Hz_index = ((OBD_sigCount_1Hz_index + 2) % (OBD_SIGCOUNT_1HZ + 2)) - 1; // increment and wait at -1
            }
            if (mode == 0x41 && pid == OBD_10Hz_ID)
            {
                OBD_sigCount_10Hz_index = ((OBD_sigCount_10Hz_index + 2) % (OBD_SIGCOUNT_10HZ + 2)) - 1; // increment and wait at -1
            }
        }

        // print to serial
        printMessageToSerialRaw(timestamp, frame, 0);

        // print to SD card
        writeCanFrameToSD(frame, timestamp, 0);
    }

    if (can1.readFrame(frame))
    {
        // log timestamps
        unsigned long long timestamp = currentTime - startTime;
        lastReceptionTime = currentTime;
        
        // print to serial
        printMessageToSerialRaw(timestamp, frame, 1);

        // print to SD card
        writeCanFrameToSD(frame, timestamp, 1);
    }

    // print CAN timeout if no frames received
    if (currentTime - lastReceptionTime > 2000000)
    {
        Serial.println("Waiting for OBD response...");
        lastReceptionTime = currentTime;
        toggleLED();
    }

    // handle user input for IO
    // TODO

    delay(50);
}
