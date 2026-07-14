#include "can.h"
#include "sd_logger.h"
#include <string.h>

xQueueHandle canRxQueue = nullptr;

static bool twai_rx_cb(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    uint8_t data[8];
    twai_frame_t rx_frame = {
        .buffer = data,
        .buffer_len = sizeof(data),
    };

    BaseType_t higher_priority_task_woken = pdFALSE;
    if (twai_node_receive_from_isr(handle, &rx_frame) == ESP_OK && canRxQueue != nullptr)
    {
        log_frame_t log_frame = {};
        log_frame.header = rx_frame.header;
        log_frame.header.timestamp = esp_timer_get_time() + epoch_offset_us;
        log_frame.bus_id = (uint8_t)(uintptr_t)user_ctx;

        // copy data from rx_frame to log_frame
        memcpy(log_frame.data, rx_frame.buffer, log_frame.header.dlc);

        // send to queue
        xQueueSendFromISR(canRxQueue, &log_frame, &higher_priority_task_woken);
    }
    return higher_priority_task_woken == pdTRUE;
}

esp_err_t setupCAN(twai_node_handle_t &bus,
                   uint8_t controllerId,
                   uint8_t sxPin,
                   uint8_t txPin,
                   uint8_t rxPin)
{
    if (canRxQueue == nullptr)
    {
        canRxQueue = xQueueCreate(CAN_RX_QUEUE_LEN, sizeof(log_frame_t));
        if (canRxQueue == nullptr)
        {
            return ESP_ERR_NO_MEM;
        }
    }
    if (sxPin != (uint8_t)-1)
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

    esp_err_t err = twai_new_node_onchip(&cfg, &bus);
    if (err != ESP_OK)
    {
        bus = nullptr;
        return err;
    }

    twai_event_callbacks_t user_cbs = {
        .on_rx_done = twai_rx_cb,
    };

    err = twai_node_register_event_callbacks(bus, &user_cbs, (void *)(uintptr_t)controllerId);
    if (err != ESP_OK)
    {
        twai_node_delete(bus);
        bus = nullptr;
        return err;
    }

    err = twai_node_enable(bus);
    if (err != ESP_OK)
    {
        twai_node_delete(bus);
        bus = nullptr;
        return err;
    }

    return ESP_OK;
}

bool CanFrameToString(const log_frame_t &log_frame, char *buf, size_t buf_size)
{
    // 1781820252108000,00000240,false,Rx,0,1,00,00,00,00,00,00,00,00,
    return snprintf(buf, buf_size, "%llu,%08X,%s,%s,%d,%d,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,", log_frame.header.timestamp,
             log_frame.header.id,
             log_frame.header.ide ? "true" : "false",
             log_frame.header.rtr ? "Tx" : "Rx",
             log_frame.bus_id,
             log_frame.header.dlc,
             log_frame.data[0],
             log_frame.data[1],
             log_frame.data[2],
             log_frame.data[3],
             log_frame.data[4],
             log_frame.data[5],
             log_frame.data[6],
             log_frame.data[7]
            ) > 0;
}

esp_err_t requestOBDSignal(twai_node_handle_t &bus, OBD_signal signal)
{
    static uint8_t data[8] = {0};
    data[0] = 0x02;            // number of additional bytes (Service ID + PID)
    data[1] = 0x01;            // service ID for current data
    data[2] = (uint8_t)signal; // PID for the requested signal

    twai_frame_t frame = {};
    frame.header.id = 0x7DF; // functional request ID for OBD2
    frame.header.dlc = 8;
    frame.buffer = data;
    frame.buffer_len = sizeof(data);

    return twai_node_transmit(bus, &frame, pdMS_TO_TICKS(1000));
}

String OBDSignalToString(OBD_signal signal)
{
    switch (signal)
    {
        case OBD_ENGINE_LOAD:
            return "OBD_ENGINE_LOAD";
        case OBD_COOLANT_TEMP:
            return "OBD_COOLANT_TEMP";
        case OBD_INTAKE_AIR_TEMP:
            return "OBD_INTAKE_AIR_TEMP";
        case OBD_MAF_FLOW_RATE:
            return "OBD_MAF_FLOW_RATE";
        case OBD_ENGINE_RPM:
            return "OBD_ENGINE_RPM";
        case OBD_VEHICLE_SPEED:
            return "OBD_VEHICLE_SPEED";
        case OBD_THROTTLE_POSITION:
            return "OBD_THROTTLE_POSITION";
        case OBD_RUNNING_TIME:
            return "OBD_RUNNING_TIME";
        case OBD_FUEL_LEVEL:
            return "OBD_FUEL_LEVEL";
        case OBD_ODOMETER:
            return "OBD_ODOMETER";
    }
    return "UNKNOWN_OBD_SIGNAL_0x" + String((uint8_t)signal, HEX);
}
