#include "can.h"

static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    uint8_t data[8];
    twai_frame_t rx_frame = {
        .buffer = data,
        .buffer_len = sizeof(data),
    };
    if (twai_node_receive_from_isr(handle, &rx_frame) == ESP_OK)
    {
        uint8_t bus_id = (uint8_t)(uintptr_t)user_ctx;
        log_frame_t log_frame = {
            .frame = rx_frame,
            .timestamp = getEpochMicroseconds(),
            .bus_id = bus_id};

        // receive ok, push to write queue
        xQueueSend(canRxQueue, &log_frame, 0);
    }
    return false;
}

esp_err_t setupCAN(twai_node_handle_t bus,
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

    twai_onchip_node_config_t cfg =
        {
            .io_cfg = {
                .tx = (gpio_num_t)txPin,
                .rx = (gpio_num_t)rxPin,
                .quanta_clk_out = (gpio_num_t)-1,
                .bus_off_indicator = (gpio_num_t)-1},
            .bit_timing = {.bitrate = 500000, .sp_permill = 800, .ssp_permill = 0},
            .fail_retry_cnt = -1,
            .tx_queue_depth = OBD_TX_LEN,
            .intr_priority = 1,
            .flags = {.enable_self_test = 0, .enable_loopback = 0, .enable_listen_only = 0, .no_receive_rtr = 0}};

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = twai_rx_cb,
    };

    twai_node_register_event_callbacks(bus, &user_cbs, (void *)(uintptr_t)controllerId);

    esp_err_t err = twai_new_node_onchip(&cfg, &bus);
    if (err != ESP_OK)
    {
        bus = nullptr;
        return err;
    }

    twai_node_enable(bus);
    if (err != ESP_OK)
    {
        twai_node_delete(bus);
        bus = nullptr;
        return err;
    }

    return ESP_OK;
}

String CanFrameToString(const twai_frame_t &frame, uint64_t timestamp, const uint8_t bus_id)
{
    char buf[70];
    // 1781820252108000,00000240,false,Rx,0,1,00,00,00,00,00,00,00,00,
    snprintf(buf, sizeof(buf), "%llu,%08X,%s,%s,%d,%d,", timestamp,
             frame.header.id,
             frame.header.ide ? "true" : "false",
             frame.header.rtr ? "Tx" : "Rx",
             bus_id,
             frame.header.dlc);
    String outString = String(buf);

    size_t data_len = frame.header.dlc;
    if (data_len > 8)
    {
        data_len = 8; // limit to 8 bytes for standard CAN frames
    }
    if (data_len > frame.buffer_len)
    {
        data_len = frame.buffer_len; // limit to available buffer length
    }

    for (size_t i = 0; i < data_len; i++)
    {
        if (frame.buffer[i] < 0x10)
            outString += "0";
        outString += String(frame.buffer[i], HEX) + ",";
    }
    for (size_t i = data_len; i < 8; i++)
    {
        outString += "00,";
    }
    return outString;
}

size_t CanFrameToCharArray(char *out, size_t out_size, const twai_frame_t &frame, uint64_t timestamp, const uint8_t bus_id)
{
    // 1781820252108000,00000240,false,Rx,0,1,00,00,00,00,00,00,00,00,
    int n = snprintf(out, out_size, "%llu,%08X,%s,%s,%d,%d,", timestamp,
                     frame.header.id,
                     frame.header.ide ? "true" : "false",
                     frame.header.rtr ? "Tx" : "Rx",
                     bus_id,
                     frame.header.dlc);

    char *p = out + n;

    static const char hex[] = "0123456789ABCDEF";
    for (size_t i = 0; i < 8; i++)
    {
        uint8_t b = (i < frame.header.dlc) ? frame.buffer[i] : 0;

        *p++ = hex[b >> 4];
        *p++ = hex[b & 0x0F];
        *p++ = ',';
    }
    *p = '\0';
    return p - out;
}

esp_err_t requestOBDSignal(twai_node_handle_t bus, OBD_signal signal)
{
    twai_frame_t frame;
    frame.header.id = 0x7DF; // functional request ID for OBD2
    frame.header.dlc = 8;
    frame.buffer[0] = 0x02;   // number of additional bytes (Service ID + PID)
    frame.buffer[1] = 0x01;   // service ID for current data
    frame.buffer[2] = signal; // PID for the requested signal
    for (size_t i = 3; i < 8; i++)
    {
        frame.buffer[i] = 0x00; // padding
    }
    return twai_node_transmit(bus, &frame, pdMS_TO_TICKS(1000));
}
