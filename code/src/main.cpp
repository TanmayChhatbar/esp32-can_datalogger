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

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

twai_node_handle_t bus0 = nullptr;
twai_node_handle_t bus1 = nullptr;
volatile int OBD_sigCount_1Hz_index = 0;
volatile int OBD_sigCount_10Hz_index = 0;
volatile OBD_signal OBD_1Hz_ID = OBD_SIGNALS_OF_INTEREST_1HZ[0];   // current OBD signal for which response is expected, -1 means no signal requested yet for delay between request batches
volatile OBD_signal OBD_10Hz_ID = OBD_SIGNALS_OF_INTEREST_10HZ[0]; // current OBD signal for which response is expected, -1 means no signal requested yet for delay between request batches
volatile unsigned long lastOBDRequestTime_1Hz_us = 0;
volatile unsigned long lastOBDRequestTime_10Hz_us = 0;
logging_mode_t logging_mode = LOGGING_MODE_WAITGPS;
bool stopOBDTask_1Hz = false;
bool stopOBDTask_10Hz = false;
bool stopIMUTask_100Hz = false;
Button but1;

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void vTask_OBD_Request_1Hz(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    while (!stopOBDTask_1Hz)
    {
        // handle queue-related timeout errors
        twai_node_status_t status_info;
        twai_node_get_info(bus1, &status_info, NULL);

        // if starting fresh, wait 1Hz equivalent time
        if (OBD_sigCount_1Hz_index == -1)
        {
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000)); // repeat every 1s
            OBD_sigCount_1Hz_index = 0;
            continue;
        }

        if (status_info.tx_queue_remaining > OBD_TX_LEN - 1)
        {
            OBD_signal target = OBD_SIGNALS_OF_INTEREST_1HZ[OBD_sigCount_1Hz_index];
            // if target changed OR 50 ms timeout for OBD response, send request for next target
            if (target != OBD_1Hz_ID || micros() - lastOBDRequestTime_1Hz_us > 500000)
            {
                OBD_1Hz_ID = target;

                esp_err_t err = requestOBDSignal(bus1, OBD_1Hz_ID);
                if (err == ESP_OK)
                {
                    lastOBDRequestTime_1Hz_us = micros();
                    Serial.println("(vTask_OBD_Request_1Hz)\tRequested OBD signal (1Hz): " + String(OBD_1Hz_ID, HEX));
                }
                else
                {
                    Serial.println("(vTask_OBD_Request_1Hz)\tOBD request failed (1Hz): " + String(OBD_1Hz_ID) + "\n\tError: " + String(esp_err_to_name(err)));
                }
            }
        }
    }

    stopOBDTask_1Hz = false;
    vTaskDelete(NULL);
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void vTask_IMU_Poll_100Hz(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    while (!stopIMUTask_100Hz)
    {
        imu_event_t imu_event = {};
        if (readIMU(imu_event))
        {
            log_frame_t imu_frame = {};
            imu_frame.header.timestamp = getEpochMicroseconds();
            imu_frame.bus_id = 2;
            if (IMUFrameToCanFrame(imu_event, imu_frame))
            {
                if (canRxQueue != nullptr)
                {
                    xQueueSend(canRxQueue, &imu_frame, 0);
                }
            }
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10)); // 100hz
    }

    stopIMUTask_100Hz = false;
    vTaskDelete(NULL);
}
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void vTask_OBD_Request_10Hz(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    while (!stopOBDTask_10Hz)
    {
        // handle queue-related timeout errors
        twai_node_status_t status_info;
        twai_node_get_info(bus1, &status_info, NULL);

        // if starting fresh, wait 1Hz equivalent time
        if (OBD_sigCount_10Hz_index == -1)
        {
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000)); // repeat every 1s
            OBD_sigCount_10Hz_index = 0;
            continue;
        }

        if (status_info.tx_queue_remaining > OBD_TX_LEN - 1)
        {
            OBD_signal target = OBD_SIGNALS_OF_INTEREST_10HZ[OBD_sigCount_10Hz_index];
            // if target changed OR 50 ms timeout for OBD response, send request for next target
            if (target != OBD_10Hz_ID || micros() - lastOBDRequestTime_10Hz_us > 500000)
            {
                OBD_10Hz_ID = target;

                esp_err_t err = requestOBDSignal(bus1, OBD_10Hz_ID);
                if (err == ESP_OK)
                {
                    lastOBDRequestTime_10Hz_us = micros();
                    Serial.println("(vTask_OBD_Request_10Hz)\tRequested OBD signal (10Hz): " + String(OBD_10Hz_ID, HEX));
                }
                else
                {
                    Serial.println("(vTask_OBD_Request_10Hz)\tOBD request failed (10Hz): " + String(OBD_10Hz_ID) + "\n\tError: " + String(esp_err_to_name(err)));
                }
            }
        }
    }

    stopOBDTask_10Hz = false;
    vTaskDelete(NULL);
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void updateLoggingMode(button_state_t user_input)
{
    if (user_input == BUTTON_SHORT_PRESS_CONFIRMED)
    {
        switch (logging_mode)
        {
        case LOGGING_MODE_INACTIVE:
        {
            logging_mode = LOGGING_MODE_START;
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
        case LOGGING_MODE_WIFI_ACTIVE:
        case LOGGING_MODE_WIFI_STOP:
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
        {
            Serial.println("Overriding GPS wait");
            logging_mode = LOGGING_MODE_INACTIVE;
            break;
        }
        case LOGGING_MODE_INACTIVE:
        {
            logging_mode = LOGGING_MODE_WIFI_START;
            break;
        }
        case LOGGING_MODE_WIFI_ACTIVE:
        {
            logging_mode = LOGGING_MODE_WIFI_STOP;
            break;
        }
        case LOGGING_MODE_WIFI_STOP:
        case LOGGING_MODE_START:
        case LOGGING_MODE_ACTIVE:
        case LOGGING_MODE_STOP:
        default:
        {
            break;
        }
        }
    }
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void setup()
{
    while (!Serial.available())
    {
        delay(100);
    }
    Serial.begin(921600);

    // set up on-board LEDs
    setupLED();

    // set up buttons
    but1.begin(BUT1, 750, false);

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
    Serial.println("\tInitialized");

    // set up IMU
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS_IMU);
    Serial.println("IMU");
    if (!setupIMU())
    {
        Serial.println("\tFailed to initialize");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.println("\tInitialized");

    // set up SD card
    pinMode(SD_CD, INPUT_PULLUP);
    Serial.println("SD");
    while (digitalRead(SD_CD))
    {
        Serial.println("\tWaiting for SD card...");
        delay(1000);
    }
    while (!SD.begin(SPI_CS_SD, SPI, 40000000))
    {
        Serial.println("\tFailed to initialize");
        delay(1000);
    }
    Serial.println("\tInitialized");

    // set up CAN buses
    esp_err_t err;
    Serial.println("bus0");
    err = setupCAN(bus0, 0, CAN0_SX, CAN0_TX, CAN0_RX);
    if (err != ESP_OK)
        Serial.printf("\tFailed: %s\n", esp_err_to_name(err));
    else
        Serial.println("\tInitialized");

    Serial.println("bus1");
    err = setupCAN(bus1, 1, CAN1_SX, CAN1_TX, CAN1_RX);
    if (err != ESP_OK)
        Serial.printf("\tFailed: %s\n", esp_err_to_name(err));
    else
        Serial.println("\tInitialized");
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void loop()
{
    // init local vars
    static File log_file;
    static unsigned long startTime_us = micros();
    static unsigned long lastRequestTime_1Hz_us = startTime_us;
    static unsigned long lastRequestTime_10Hz_us = startTime_us;
    static unsigned long ledTime_ms = millis();
    static bool wait_gps_print = false;
    static unsigned long wait_gps_start_ms = millis();
    static unsigned long counter = 0;
    static unsigned long lastCycleTime_us = micros();

    // logging actions
    switch (logging_mode)
    {
    case LOGGING_MODE_WAITGPS:
    {
        // wait for fix
        if (!wait_gps_print)
        {
            Serial.println("GPS\n\tWaiting for GPS fix");
            wait_gps_print = true;
            wait_gps_start_ms = millis();
        }
        if (millis() - wait_gps_start_ms > 10000)
        {
            Serial.print('.');
            wait_gps_start_ms = millis();
        }

        readGPS();

        if (GPS.fix)
        {
            Serial.println("\tFix acquired");
            logging_mode = LOGGING_MODE_INACTIVE;
            syncTimeFromGPS();
        }
        break;
    }
    case LOGGING_MODE_INACTIVE:
    {
        // TODO keep checking sensor state
        break;
    }

    case LOGGING_MODE_WIFI_START:
    {
        // TODO
        logging_mode = LOGGING_MODE_WIFI_ACTIVE;
        break;
    }
    case LOGGING_MODE_WIFI_ACTIVE:
    {
        // TODO
        break;
    }
    case LOGGING_MODE_WIFI_STOP:
    {
        // TODO
        logging_mode = LOGGING_MODE_INACTIVE;
        break;
    }

    case LOGGING_MODE_START:
    {
        // filename
        const char *filename = getFilenameForCurrentTime();
        log_file = SD.open(filename, FILE_WRITE); // TODO takes more than 0.7 seconds
        Serial.print("Logging to file: ");
        Serial.println(filename);

        // start logging
        writeCANHeaderToSD(log_file);
        logging_mode = LOGGING_MODE_ACTIVE;

        // init vars
        startTime_us = micros() - GPS.milliseconds * 1000; // so that timestamps are relative to the GPS time to the second
        lastRequestTime_1Hz_us = startTime_us;
        lastRequestTime_10Hz_us = startTime_us;

        // start OBD request tasks
        xTaskCreatePinnedToCore(vTask_OBD_Request_1Hz, "OBD_Request_1Hz", 4096, NULL, 1, NULL, 0);
        // xTaskCreatePinnedToCore(vTask_OBD_Request_10Hz, "OBD_Request_10Hz", 4096, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(vTask_IMU_Poll_100Hz, "IMU_Poll_100Hz", 4096, NULL, 1, NULL, 0);

        break;
    }

    case LOGGING_MODE_ACTIVE:
    {
        unsigned long currentTime_us = micros();

        if (canRxQueue != nullptr)
        {
            log_frame_t log_frame;
            if (xQueueReceive(canRxQueue, &log_frame, 0) == pdTRUE)
            {
                // OBD response handling
                if (log_frame.bus_id == 1 && log_frame.header.id >= 0x7E8 && log_frame.header.id <= 0x7EF && log_frame.header.dlc > 2)
                {
                    uint8_t mode = log_frame.data[1]; // should be 0x41 for response to 01
                    uint8_t pid = log_frame.data[2];
                    if (mode == 0x41)
                    {
                        if (pid == OBD_1Hz_ID)
                        {
                            OBD_sigCount_1Hz_index = ((OBD_sigCount_1Hz_index + 2) % (OBD_SIGCOUNT_1HZ + 2)) - 1; // increment and wait at -1
                        }
                        if (pid == OBD_10Hz_ID)
                        {
                            OBD_sigCount_10Hz_index = ((OBD_sigCount_10Hz_index + 2) % (OBD_SIGCOUNT_10HZ + 2)) - 1; // increment and wait at -1
                        }
                    }
                }

                // print to SD card
                writeCanFrameToSD(log_frame, log_file);
            }
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
            log_frame_t gps_frame[4] = {};
            for (int i = 0; i < 4; i++)
            {
                gps_frame[i].header.timestamp = timestamp;
                gps_frame[i].bus_id = 2;
            }
            if (GPSFrameToCanFrame(gps_frame))
            {
                for (int i = 0; i < 4; i++)
                {
                    xQueueSend(canRxQueue, &gps_frame[i], 0);
                }
            }
        }

        break;
    }

    case LOGGING_MODE_STOP:
    {
        // stop OBD request tasks
        stopOBDTask_1Hz = true;
        stopOBDTask_10Hz = true;
        stopIMUTask_100Hz = true;

        // flush can rx queue
        log_frame_t log_frame;
        unsigned long flush_time = millis();
        while (xQueueReceive(canRxQueue, &log_frame, 0) == pdTRUE && millis() - flush_time < 1000)
        {
            writeCanFrameToSD(log_frame, log_file);
        }
        if (canRxQueue != nullptr)
        {
            xQueueReset(canRxQueue);
        }
        
        // close file
        Serial.print("Logging stopped: ");
        Serial.println(log_file.path());
        log_file.flush();
        log_file.close();

        logging_mode = LOGGING_MODE_INACTIVE;
        break;
    }
    case LOGGING_MODE_TEST:
    {
        break;
    }
    default:
    {
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
    updateLoggingMode(user_input);

    // measure cycle freq
    counter++;
    if (micros() - lastCycleTime_us > 1000000)
    {
        Serial.println("Loop freq: " + String(counter) + " Hz");
        counter = 0;
        lastCycleTime_us = micros();
    }
}
