#include <Arduino.h>
#include <FastLED.h>
#include "pindef.h"
#include "led.h"
#include "can.h"
#include "canmod_gps.h"
#include "sd_logger.h"
#include "imu.h"
#include "gps.h"
#include "button.hpp"

twai_handle_t can0 = nullptr;
twai_handle_t can1 = nullptr;
volatile int OBD_sigCount_1Hz_index = 0;
volatile int OBD_sigCount_10Hz_index = 0;
volatile OBD_signal OBD_1Hz_ID = OBD_SIGNALS_OF_INTEREST_1HZ[0];   // current OBD signal for which response is expected, -1 means no signal requested yet for delay between request batches
volatile OBD_signal OBD_10Hz_ID = OBD_SIGNALS_OF_INTEREST_10HZ[0]; // current OBD signal for which response is expected, -1 means no signal requested yet for delay between request batches
volatile unsigned long long lastOBDRequestTime_1Hz_us = 0;
volatile unsigned long long lastOBDRequestTime_10Hz_us = 0;
logging_mode_t logging_mode = LOGGING_MODE_WAITGPS;
bool stopOBDTask_1Hz = false;
bool stopOBDTask_10Hz = false;
Button but1;

void vTask_OBD_Request_1Hz(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    while (!stopOBDTask_1Hz)
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
        if (target != OBD_1Hz_ID || micros() - lastOBDRequestTime_1Hz_us > 500000)
        {
            OBD_1Hz_ID = target;

            esp_err_t err = requestOBDSignal(can1, OBD_1Hz_ID);
            if (err != ESP_OK)
            {
                Serial.println("(vTask_OBD_Request_1Hz)\tOBD request failed (1Hz): " + String(OBD_1Hz_ID) + "\n\tError: " + String(esp_err_to_name(err)));
            }
            else
            {
                lastOBDRequestTime_1Hz_us = micros();
                Serial.println("(vTask_OBD_Request_1Hz)\tRequested OBD signal (1Hz): " + String(OBD_1Hz_ID, HEX));
            }
            
            // TODO handle queue-related timeout errors
        }
    }

    stopOBDTask_1Hz = false;
    vTaskDelete(NULL);
}

void vTask_OBD_Request_10Hz(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    while (!stopOBDTask_10Hz)
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
        if (target != OBD_10Hz_ID || micros() - lastOBDRequestTime_10Hz_us > 500000)
        {
            OBD_10Hz_ID = target;

            esp_err_t err = requestOBDSignal(can1, OBD_10Hz_ID);
            if (err != ESP_OK)
            {
                Serial.println("(vTask_OBD_Request_10Hz)\tOBD request failed (10Hz): " + String(OBD_10Hz_ID) + "\n\tError: " + String(esp_err_to_name(err)));
            }
            else
            {
                lastOBDRequestTime_10Hz_us = micros();
                Serial.println("(vTask_OBD_Request_10Hz)\tRequested OBD signal (10Hz): " + String(OBD_10Hz_ID, HEX));
            }
        }
    }

    stopOBDTask_10Hz = false;
    vTaskDelete(NULL);
}

void setup()
{
    while (!Serial.available())
    {
        delay(100);
    }
    Serial.begin(460800);

    // set up on-board LEDs
    setupLED();

    // set up buttons
    but1.begin(BUT1, 500, false);

    // set up GPS
    Serial.println("GPS");
    if (!setupGPS())
    {
        Serial.println("\tFailed to initialize");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.println("\tInitialized successfully");

    // set up IMU
    Serial.println("IMU");
    if (!setupIMU())
        Serial.println("\tTODO\n\tFailed to initialize");

    // set up SD card
    pinMode(SD_CD, INPUT_PULLUP);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS_SD);
    Serial.println("SD");
    while (digitalRead(SD_CD))
    {
        Serial.println("\tWaiting for SD card...");
        delay(1000);
    }
    while (!SD.begin(SPI_CS_SD, SPI, 400000))
    {
        Serial.println("\tFailed to initialize");
        delay(1000);
    }
    Serial.println("\tInitialized");

    // set up CAN buses
    Serial.println("CAN0");
    esp_err_t err = setupCAN(can0, 0, CAN0_SX, CAN0_TX, CAN0_RX);
    if (err != ESP_OK)
        Serial.printf("\tTWAI%d install failed: %s\n", 0, esp_err_to_name(err));
    else
        Serial.printf("\tTWAI%d started on TX=%d RX=%d\n", 0, CAN0_TX, CAN0_RX);

    Serial.println("CAN1");
    err = setupCAN(can1, 1, CAN1_SX, CAN1_TX, CAN1_RX);
    if (err != ESP_OK)
        Serial.printf("\tTWAI%d install failed: %s\n", 1, esp_err_to_name(err));
    else
        Serial.printf("\tTWAI%d started on TX=%d RX=%d\n", 1, CAN1_TX, CAN1_RX);
}

void loop()
{
    // init local vars
    static File log_file;
    static unsigned long long startTime_us = micros();
    static unsigned long long lastReceptionTime_us = startTime_us;
    static unsigned long long lastRequestTime_1Hz_us = startTime_us;
    static unsigned long long lastRequestTime_10Hz_us = startTime_us;
    static unsigned long long currentTime_us = startTime_us;
    static unsigned long ledTime_ms = millis();

    switch (logging_mode)
    {
    case LOGGING_MODE_WAITGPS:
    {
        // wait for fix
        Serial.println("\tWaiting for GPS fix");
        while (!waitForGPSFix(5000))
        {
            Serial.println("\tStill waiting");
        }
        Serial.println("\tFix acquired");
        syncTimeFromGPS();
        logging_mode = LOGGING_MODE_INACTIVE;
        break;
    }
    case LOGGING_MODE_INACTIVE:
    default:
    {
        // TODO keep checking sensor state
        break;
    }

    case LOGGING_MODE_WIFI:
    {
        // TODO
        break;
    }
    case LOGGING_MODE_STOP_WIFI:
    {
        // TODO
        break;
    }

    case LOGGING_MODE_START:
    {
        // new file
        // get time from GPS

        // filename
        const char *filename = getFilenameForCurrentTime();
        log_file = SD.open(filename, FILE_WRITE);
        Serial.print("Logging to file: ");
        Serial.println(filename);

        // start logging
        writeCANHeaderToSD(log_file);
        logging_mode = LOGGING_MODE_ACTIVE;

        // init vars
        startTime_us = micros() - GPS.milliseconds * 1000; // so that timestamps are relative to the GPS time to the second
        lastReceptionTime_us = startTime_us;
        lastRequestTime_1Hz_us = startTime_us;
        lastRequestTime_10Hz_us = startTime_us;
        currentTime_us = startTime_us;

        // start OBD request tasks
        xTaskCreatePinnedToCore(vTask_OBD_Request_1Hz, "OBD_Request_1Hz", 4096, NULL, 1, NULL, 0);
        // xTaskCreatePinnedToCore(vTask_OBD_Request_10Hz, "OBD_Request_10Hz", 4096, NULL, 1, NULL, 0);

        break;
    }

    case LOGGING_MODE_ACTIVE:
    {
        currentTime_us = micros();

        twai_message_t frame;
        if (twai_receive_v2(can0, &frame, pdMS_TO_TICKS(2)) == ESP_OK)
        {
            // log timestamps
            unsigned long long timestamp = getEpochMicroseconds();
            lastReceptionTime_us = currentTime_us;

            // print to SD card
            writeCanFrameToSD(frame, timestamp, 0, log_file);
        }

        if (twai_receive_v2(can1, &frame, pdMS_TO_TICKS(2)) == ESP_OK)
        {
            // log timestamps
            unsigned long long timestamp = getEpochMicroseconds();
            lastReceptionTime_us = currentTime_us;

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

            // print to SD card
            writeCanFrameToSD(frame, timestamp, 1, log_file);
        }

        // move to next OBD signal request if timeout
        if (currentTime_us - lastRequestTime_1Hz_us > 100000)
        {
            OBD_sigCount_1Hz_index = ((OBD_sigCount_1Hz_index + 2) % (OBD_SIGCOUNT_1HZ + 2)) - 1; // increment and wait at -1
            lastRequestTime_1Hz_us = currentTime_us;
        }
        if (currentTime_us - lastRequestTime_10Hz_us > 100000)
        {
            OBD_sigCount_10Hz_index = ((OBD_sigCount_10Hz_index + 2) % (OBD_SIGCOUNT_10HZ + 2)) - 1; // increment and wait at -1
            lastRequestTime_10Hz_us = currentTime_us;
        }

        // (virtual CAN) GPS
        if (readGPS())
        {
            unsigned long long timestamp = getEpochMicroseconds();
            twai_message_t gps_frame[4];
            if (GPSFrameToCanFrame(gps_frame))
            {
                for (int i = 0; i < 4; i++)
                {
                    writeCanFrameToSD(gps_frame[i], timestamp, 2, log_file);
                }
            }
        }

        // TODO (virtual CAN) IMU

        // print CAN timeout if no frames received
        if (currentTime_us - lastReceptionTime_us > 2000000)
        {
            Serial.println("Waiting for OBD response...");
            lastReceptionTime_us = currentTime_us;
        }

        break;
    }

    case LOGGING_MODE_STOP:
    {
        // close file
        Serial.print("Logging stopped: ");
        Serial.println(log_file.path());
        log_file.close();

        // stop OBD request tasks
        stopOBDTask_1Hz = true;
        stopOBDTask_10Hz = true;

        logging_mode = LOGGING_MODE_INACTIVE;
        break;
    }
    }

    // handle LED
    if (handleLED(logging_mode, millis() - ledTime_ms))
    {
        ledTime_ms = millis();
    }

    // handle user input
    button_state_t user_input = but1.readButtonState();

    if (user_input == BUTTON_SHORT_PRESS_CONFIRMED)
    {
        switch (logging_mode)
        {
        case LOGGING_MODE_INACTIVE:
        {
            logging_mode = LOGGING_MODE_START;
            unsigned long long button_time = millis();
            while (!digitalRead(BUT1))
            {
                if (millis() - button_time > 1000)
                {
                    logging_mode = LOGGING_MODE_WIFI;
                    Serial.println("Enabling WiFi");
                    break;
                }
                delay(50);
            }
            if (logging_mode == LOGGING_MODE_START)
            {
                Serial.println("Logging started");
            }
            break;
        }

        case LOGGING_MODE_ACTIVE:
        {
            logging_mode = LOGGING_MODE_STOP;
            break;
        }
        case LOGGING_MODE_WAITGPS:
        case LOGGING_MODE_STOP:
        case LOGGING_MODE_START:
        case LOGGING_MODE_WIFI:
        case LOGGING_MODE_STOP_WIFI:
        default:
        {
            break;
        }
        }
    }
    else if (user_input == BUTTON_LONG_PRESS_CONFIRMED)
    {
        switch (logging_mode)
        {
        case LOGGING_MODE_WAITGPS:
        case LOGGING_MODE_INACTIVE:
        {
            logging_mode = LOGGING_MODE_WIFI;
            break;
        }
        case LOGGING_MODE_WIFI:
        {
            logging_mode = LOGGING_MODE_STOP_WIFI;
            break;
        }
        case LOGGING_MODE_START:
        case LOGGING_MODE_ACTIVE:
        case LOGGING_MODE_STOP:
        {
            break;
        }
        }
    }
    delay(50);
}
