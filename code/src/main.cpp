#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "defs.h"
#include "led.h"
#include "can.h"
#include "canmod_gps.h"
#include "sd_logger.h"
#include "imu.h"
#include "gps.h"
#include "button.hpp"
#include "wifi_utils.h"
#include <ArduinoOTA.h>

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
unsigned long long epoch_offset_us = 0;
volatile logging_mode_t logging_mode = LOGGING_MODE_SETUP;
volatile bool stopOBDTask_1Hz = false;
volatile bool stopOBDTask_10Hz = false;
volatile bool stopIMUTask_100Hz = false;
Button but1;
unsigned long led_time_ms = millis();

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
            lastOBDRequestTime_1Hz_us = micros();
            continue;
        }

        if (status_info.tx_queue_remaining > 0)
        {
            if (micros() - lastOBDRequestTime_1Hz_us > 50000)
            {
                OBD_sigCount_1Hz_index = ((OBD_sigCount_1Hz_index + 2) % (OBD_SIGCOUNT_1HZ + 1)) - 1;
                if (OBD_sigCount_1Hz_index == -1)
                {
                    continue;
                }
            }

            // if target changed, send request for next target
            OBD_signal target = OBD_SIGNALS_OF_INTEREST_1HZ[OBD_sigCount_1Hz_index];
            if (target != OBD_1Hz_ID)
            {
                OBD_1Hz_ID = target;

                esp_err_t err = requestOBDSignal(bus1, OBD_1Hz_ID);
                if (err == ESP_OK)
                {
                    lastOBDRequestTime_1Hz_us = micros();
#if SERIAL_OBD_REQUESTS
                    Serial.println("(vT_1Hz)\tRequested: " + OBDSignalToString(OBD_1Hz_ID));
#endif
                }
                else
                {
                    Serial.println("(vT_1Hz)\tRequest failed: " + OBDSignalToString(OBD_1Hz_ID) + "\n\tError: " + String(esp_err_to_name(err)));
                }
            }
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(5)); // repeat every 5ms
    }

    stopOBDTask_1Hz = false;
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
            vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100)); // repeat every 100ms
            OBD_sigCount_10Hz_index = 0;
            lastOBDRequestTime_10Hz_us = micros();
            continue;
        }

        if (status_info.tx_queue_remaining > 0)
        {
            if (micros() - lastOBDRequestTime_10Hz_us > 10000)
            {
                OBD_sigCount_10Hz_index = ((OBD_sigCount_10Hz_index + 2) % (OBD_SIGCOUNT_10HZ + 1)) - 1;
                if (OBD_sigCount_10Hz_index == -1)
                {
                    continue;
                }
            }

            // if target changed, send request for next target
            OBD_signal target = OBD_SIGNALS_OF_INTEREST_10HZ[OBD_sigCount_10Hz_index];
            if (target != OBD_10Hz_ID)
            {
                OBD_10Hz_ID = target;

                esp_err_t err = requestOBDSignal(bus1, OBD_10Hz_ID);
                if (err == ESP_OK)
                {
                    lastOBDRequestTime_10Hz_us = micros();
#if SERIAL_OBD_REQUESTS
                    Serial.println("(vT_10Hz)\tRequested: " + OBDSignalToString(OBD_10Hz_ID));
#endif
                }
                else
                {
                    Serial.println("(vT_10Hz)\tRequest failed: " + OBDSignalToString(OBD_10Hz_ID) + "\n\tError: " + String(esp_err_to_name(err)));
                }
            }
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(5)); // repeat every 5ms
    }

    stopOBDTask_10Hz = false;
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
            log_frame_t imu_frames[2] = {};
            unsigned long long timestamp_us = esp_timer_get_time();
            for (int i = 0; i < 2; i++)
            {
                imu_frames[i].header.timestamp = timestamp_us + epoch_offset_us;
                imu_frames[i].bus_id = 2;
            }
            if (IMUFrameToCanFrame(imu_event, imu_frames))
            {
                if (canRxQueue != nullptr)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        xQueueSend(canRxQueue, &imu_frames[i], 0);
                    }
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

void vTask_LED_handler(void *parameter)
{
    TickType_t lastWake = xTaskGetTickCount();

    while (true)
    {
        lastWake = xTaskGetTickCount();
        if (handleLED(logging_mode, millis() - led_time_ms))
        {
            led_time_ms = millis();
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(50));
    }
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void updateLoggingMode(button_state_t user_input)
{
    button_press_t state = user_input.state;
    int button_index = user_input.button_index;
    if (button_index == -1)
    {
        return;
    }
    else if (button_index == 0)
    {
        if (state == BUTTON_SHORT_PRESS_CONFIRMED)
        {
            switch (logging_mode)
            {
            case LOGGING_MODE_INACTIVE:
            {
                logging_mode = LOGGING_MODE_START;
                break;
            }

            case LOGGING_MODE_ACTIVE:
            case LOGGING_MODE_ACTIVE_CAN:
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
        else if (state == BUTTON_LONG_PRESS_CONFIRMED)
        {
            switch (logging_mode)
            {
            case LOGGING_MODE_WAITGPS:
            case LOGGING_MODE_INACTIVE:
            case LOGGING_MODE_WIFI_ACTIVE:
            case LOGGING_MODE_WIFI_STOP:
            case LOGGING_MODE_START:
            case LOGGING_MODE_ACTIVE:
            case LOGGING_MODE_ACTIVE_CAN:
            case LOGGING_MODE_STOP:
            default:
            {
                break;
            }
            }
        }
    }
    else if (button_index == 1)
    {
        if (state == BUTTON_SHORT_PRESS_CONFIRMED)
        {
            // nothing
        }
        else if (state == BUTTON_LONG_PRESS_CONFIRMED)
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
                Serial.println("Stopping WiFi");
                logging_mode = LOGGING_MODE_WIFI_STOP;
                break;
            }
            case LOGGING_MODE_WIFI_STOP:
            case LOGGING_MODE_START:
            case LOGGING_MODE_ACTIVE:
            case LOGGING_MODE_ACTIVE_CAN:
            case LOGGING_MODE_STOP:
            default:
            {
                break;
            }
            }
        }
    }
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

String esp_reset_reason_to_name(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_UNKNOWN:
        return "ESP_RST_UNKNOWN";
    case ESP_RST_POWERON:
        return "ESP_RST_POWERON";
    case ESP_RST_EXT:
        return "ESP_RST_EXT";
    case ESP_RST_SW:
        return "ESP_RST_SW";
    case ESP_RST_PANIC:
        return "ESP_RST_PANIC";
    case ESP_RST_INT_WDT:
        return "ESP_RST_INT_WDT";
    case ESP_RST_TASK_WDT:
        return "ESP_RST_TASK_WDT";
    case ESP_RST_WDT:
        return "ESP_RST_WDT";
    case ESP_RST_DEEPSLEEP:
        return "ESP_RST_DEEPSLEEP";
    case ESP_RST_BROWNOUT:
        return "ESP_RST_BROWNOUT";
    case ESP_RST_SDIO:
        return "ESP_RST_SDIO";
    case ESP_RST_USB:
        return "ESP_RST_USB";
    case ESP_RST_JTAG:
        return "ESP_RST_JTAG";
    case ESP_RST_EFUSE:
        return "ESP_RST_EFUSE";
    case ESP_RST_PWR_GLITCH:
        return "ESP_RST_PWR_GLITCH";
    case ESP_RST_CPU_LOCKUP:
        return "ESP_RST_CPU_LOCKUP";
    default:
        return "Unknown reset reason";
    }
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void setup()
{
    Serial.begin(921600);
    Serial.setTxTimeoutMs(0);

    // set up on-board LEDs
    setupLED();
    xTaskCreatePinnedToCore(vTask_LED_handler, "LED_handler", 2048, NULL, 1, NULL, 0);

    // set up buttons TODO multi button
    but1.begin(BUTS, 750, NUMBUTS, BUTS_V);

    // set up GPS
    Serial.println("GPS");
    while (!setupGPS())
    {
        Serial.println("\tFailed to initialize");
        delay(1000);
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
        button_state_t state = but1.readButtonState();
        if (state.button_index == 0 && state.state == BUTTON_LONG_PRESS_CONFIRMED)
        {
            Serial.println("\tOverriding SD card detection");
            break;
        }
        delay(1000);
    }
    while (!SD.begin(SPI_CS_SD, SPI, 40000000))
    {
        button_state_t state = but1.readButtonState();
        if (state.button_index == 0 && state.state == BUTTON_LONG_PRESS_CONFIRMED)
        {
            Serial.println("\tOverriding SD card detection");
            break;
        }
        if (state.button_index == 0 && state.state == BUTTON_LONG_PRESS_CONTINUED)
        {
            break;
        }
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

    // log crash reason
    esp_reset_reason_t reason = esp_reset_reason();
    SD.mkdir("/crash_logs");
    File crash_log = SD.open("/crash_logs/crash_log.txt", FILE_APPEND);
    if (crash_log)
    {
        crash_log.printf("Reset reason: ");
        crash_log.println(esp_reset_reason_to_name(reason));
        crash_log.close();
    }

    // move to wait for GPS fix
    logging_mode = LOGGING_MODE_WAITGPS;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void loop()
{
    // init local vars
    static File log_file;
    static bool wait_gps_print = false;
    static unsigned long wait_gps_start_ms = millis();
    static bool last_gps_fix_state = false;
    static bool gps_fix_var = true;
    static unsigned long lastOBDReceiveTime_ms = wait_gps_start_ms;

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
        if (millis() - wait_gps_start_ms > 1200)
        {
            Serial.println("\tFix acquired");
            vTaskDelay(pdMS_TO_TICKS(100));
            while (!readGPS())
            {
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            if (GPS.year > 25 && GPS.year < 30)
            {
                syncTimeFromGPS();
                logging_mode = LOGGING_MODE_INACTIVE;
            }
            else
            {
                Serial.println("\tGPS time invalid (" + String(GPS.year) + ")");
                // esp_restart();
            }
        }

        gps_fix_var = digitalRead(GPS_FIX);
        if (gps_fix_var != last_gps_fix_state)
        {
            last_gps_fix_state = gps_fix_var;
            wait_gps_start_ms = millis();
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
        bool status = wifiEnable();
        if (!status)
        {
            status = wifiEnableAP();
        }
        if (status)
        {
            logging_mode = LOGGING_MODE_WIFI_ACTIVE;
            ArduinoOTA.setHostname("esp32-ota");
            ArduinoOTA.setPassword("admin");
            ArduinoOTA.begin();
        }
        else
        {
            Serial.println("Returning to Inactive");
            logging_mode = LOGGING_MODE_INACTIVE;
        }
        break;
    }
    case LOGGING_MODE_WIFI_ACTIVE:
    {
        server.handleClient();
        ArduinoOTA.handle();
        delay(2); // allow the cpu to switch to other tasks
        break;
    }
    case LOGGING_MODE_WIFI_STOP:
    {
        wifiDisable();
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
        epoch_offset_us = getEpochMicroseconds() - esp_timer_get_time(); // offset between esp-timer and epoch time at start of logging, so frame-timestamp + epoch_offset_us = epoch time
        xQueueReset(canRxQueue);                                         // reset queue so no frames are logged with false timestamps

        // start OBD request tasks
        xTaskCreatePinnedToCore(vTask_OBD_Request_1Hz, "OBD_1Hz", 4096, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(vTask_OBD_Request_10Hz, "OBD_10Hz", 4096, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(vTask_IMU_Poll_100Hz, "IMU_100Hz", 4096, NULL, 1, NULL, 0);

        break;
    }

    case LOGGING_MODE_ACTIVE:
    case LOGGING_MODE_ACTIVE_CAN:
    {
        unsigned long currentTime_us = micros();
        unsigned long currentTime_ms = millis();

        if (canRxQueue != nullptr)
        {
            log_frame_t log_frame = {};
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
                            OBD_sigCount_1Hz_index = ((OBD_sigCount_1Hz_index + 2) % (OBD_SIGCOUNT_1HZ + 1)) - 1; // increment and wait at -1
                            overrideLED(GRB_YELLOW);
                            logging_mode = LOGGING_MODE_ACTIVE_CAN;
                            lastOBDReceiveTime_ms = currentTime_ms;
#if SERIAL_OBD_REQUESTS
                            Serial.println("\t\tReceived: " + OBDSignalToString(OBD_1Hz_ID));
#endif
                        }
                        if (pid == OBD_10Hz_ID)
                        {
                            OBD_sigCount_10Hz_index = ((OBD_sigCount_10Hz_index + 2) % (OBD_SIGCOUNT_10HZ + 1)) - 1; // increment and wait at -1
                            overrideLED(GRB_YELLOW);
                            logging_mode = LOGGING_MODE_ACTIVE_CAN;
                            lastOBDReceiveTime_ms = currentTime_ms;
#if SERIAL_OBD_REQUESTS
                            Serial.println("\t\tReceived: " + OBDSignalToString(OBD_10Hz_ID));
#endif
                        }
                    }
                }

                // print to SD card
                writeCanFrameToSD(log_frame, log_file);
            }
        }

        // if no OBD responses received for 5 seconds, switch to active mode without CAN
        if (logging_mode == LOGGING_MODE_ACTIVE_CAN && currentTime_ms - lastOBDReceiveTime_ms > 5000)
        {
            logging_mode = LOGGING_MODE_ACTIVE;
        }

        // (virtual CAN) GPS
        if (readGPS())
        {
            unsigned long long timestamp_us = esp_timer_get_time();
            log_frame_t gps_frame[4] = {};
            for (int i = 0; i < 4; i++)
            {
                gps_frame[i].header.timestamp = timestamp_us + epoch_offset_us;
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
        log_frame_t log_frame = {};
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
        log_file.flush();
        size_t log_file_size = log_file.size();
        Serial.print("Logging stopped: ");
        Serial.print(log_file.path());
        Serial.print(" (");
        Serial.print(log_file_size);
        Serial.print(" bytes) ");
        if (log_file_size > 1024 * 1024)
        {
            Serial.print("(");
            Serial.print(log_file_size / (1024 * 1024));
            Serial.println(" MB)");
        }
        else if (log_file_size > 1024)
        {
            Serial.print("(");
            Serial.print(log_file_size / 1024);
            Serial.println(" kB)");
        }
        else
        {
            Serial.println();
        }
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

    // handle user input
    button_state_t user_input = but1.readButtonState();
    updateLoggingMode(user_input);

    // handle commands over serial
#if SERIAL_COMMANDS
    if (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (strcmp(cmd.c_str(), "s") == 0)
        {
            if (logging_mode == LOGGING_MODE_INACTIVE)
            {
                logging_mode = LOGGING_MODE_START;
            }
            else if (logging_mode == LOGGING_MODE_ACTIVE || logging_mode == LOGGING_MODE_ACTIVE_CAN)
            {
                logging_mode = LOGGING_MODE_STOP;
            }
        }
    }
#endif
}
