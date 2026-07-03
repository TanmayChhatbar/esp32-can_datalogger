#include "can.h"

#include <Arduino.h>

esp_err_t setupCAN(twai_handle_t &bus,
                   uint8_t controllerId,
                   uint8_t sxPin,
                   uint8_t txPin,
                   uint8_t rxPin)
{
    if (sxPin >= 0)
    {
        pinMode(sxPin, OUTPUT);
        digitalWrite(sxPin, LOW);
    }

    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)txPin,
                                    (gpio_num_t)rxPin,
                                    TWAI_MODE_NORMAL);

    g_config.controller_id = controllerId;
    g_config.tx_queue_len = OBD_TX_LEN;
    g_config.rx_queue_len = 20;
    g_config.alerts_enabled = TWAI_ALERT_NONE;

    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t err = twai_driver_install_v2(&g_config, &t_config, &f_config, &bus);
    if (err != ESP_OK)
    {
        bus = nullptr;
        return err;
    }

    err = twai_start_v2(bus);
    if (err != ESP_OK)
    {
        twai_driver_uninstall_v2(bus);
        bus = nullptr;
        return err;
    }

    return ESP_OK;
}

String CanFrameToString(const twai_message_t &frame, uint64_t timestamp, const uint8_t bus)
{
    char buf[70];
    snprintf(buf, sizeof(buf), "%llu,%08X,%s,%s,%d,%d,", timestamp,
                                                        frame.identifier,
                                                        frame.extd ? "true" : "false",
                                                        frame.rtr ? "Tx" : "Rx",
                                                        bus,
                                                        frame.data_length_code);
    String outString = String(buf);
    for (int i = 0; i < frame.data_length_code; i++)
    {
        if (frame.data[i] < 0x10)
            outString += "0";
        outString += String(frame.data[i], HEX) + ",";
    }
    for (int i = frame.data_length_code; i < 8; i++)
    {
        outString += "00,";
    }
    return outString;
}

esp_err_t requestOBDSignal(twai_handle_t &can, OBD_signal signal)
{
    twai_message_t frame;
    frame.identifier = 0x7DF; // functional request ID for OBD2
    frame.data_length_code = 8;
    frame.data[0] = 0x02;   // number of additional bytes (Service ID + PID)
    frame.data[1] = 0x01;   // service ID for current data
    frame.data[2] = signal; // PID for the requested signal
    for (int i = 3; i < 8; i++)
    {
        frame.data[i] = 0x00; // padding
    }
    return twai_transmit_v2(can, &frame, pdMS_TO_TICKS(1000));
}
