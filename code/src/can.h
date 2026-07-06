#pragma once
#include <Arduino.h>
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "esp_twai_types.h"
#include "esp_err.h"
#include "sd_logger.h"
// https://docs.espressif.com/projects/esp-idf/en/v6.0.2/esp32c6/api-reference/peripherals/twai.html

enum OBD_signal : uint8_t
{
    OBD_ENGINE_LOAD = 0x04,
    OBD_COOLANT_TEMP = 0x05,
    OBD_INTAKE_AIR_TEMP = 0x06,
    OBD_MAF_FLOW_RATE = 0x07,
    OBD_ENGINE_RPM = 0x0C,
    OBD_VEHICLE_SPEED = 0x0D,
    OBD_THROTTLE_POSITION = 0x11,
    OBD_RUNNING_TIME = 0x1F,
    OBD_FUEL_LEVEL = 0x2F,
    OBD_ODOMETER = 0xA6
};

struct log_frame_t
{
    twai_frame_t frame;
    unsigned long long timestamp;
    uint8_t bus_id;
};

#define OBD_TX_LEN 10

const OBD_signal OBD_SIGNALS_OF_INTEREST_1HZ[] = {OBD_FUEL_LEVEL, OBD_RUNNING_TIME, OBD_ODOMETER};
const OBD_signal OBD_SIGNALS_OF_INTEREST_10HZ[] = {OBD_ENGINE_LOAD, OBD_VEHICLE_SPEED};

esp_err_t setupCAN(twai_node_handle_t &bus, uint8_t controllerId, uint8_t sxPin, uint8_t txPin, uint8_t rxPin);
esp_err_t requestOBDSignal(twai_node_handle_t &bus, OBD_signal signal);
String CanFrameToString(const twai_message_t &frame, const unsigned long long timestamp, const uint8_t bus_id);
size_t CanFrameToCharArray(char *out, size_t out_size, const twai_message_t &frame, uint64_t timestamp, const uint8_t bus_id);
const uint8_t OBD_SIGCOUNT_1HZ = sizeof(OBD_SIGNALS_OF_INTEREST_1HZ) / sizeof(OBD_signal);
const uint8_t OBD_SIGCOUNT_10HZ = sizeof(OBD_SIGNALS_OF_INTEREST_10HZ) / sizeof(OBD_signal);
xQueueHandle canRxQueue;
