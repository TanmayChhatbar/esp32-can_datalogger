#include "tpmss.h"

BLEScan *pBLEScan = nullptr;
const int scanTime = 1; // [s]
unsigned long long last_scan_time = 0;
tpms_t tpms[TPMS_COUNT] = {
    {"37:39:0e:10:cd:5e", "Front", 0, 0.0, 0.0},
    {"37:39:0e:10:b8:a7", "Rear", 1, 0.0, 0.0},
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice dev)
    {
        for (int i = 0; i < TPMS_COUNT; i++)
        {
            if (dev.getAddress().toString() == tpms[i].id)
            {
                // manufacturer data contains the pressure and temperature data
                String data = dev.getManufacturerData();
                if (data.length() > 4)
                {
                    // decode and store
                    tpms[i].pressure = (float)((uint8_t)data[3] << 8 | (uint8_t)data[4]) / 10.0;
                    tpms[i].temperature = (float)((uint8_t)data[2]);
                    Serial.printf("TPMS %s\tPressure: %.1f psi\tTemperature: %.1f degC\n", tpms[i].name, tpms[i].pressure, tpms[i].temperature);

                    log_frame_t tpms_frame = {};
                    unsigned long long timestamp_us = esp_timer_get_time();
                    tpms_frame.header.timestamp = timestamp_us + epoch_offset_us;
                    tpms_frame.bus_id = 2;

                    if (TPMSFrameToCanFrame(tpms[i], tpms_frame) && canRxQueue != nullptr)
                    {
                        xQueueSend(canRxQueue, &tpms_frame, 0);
                    }
                }

                return;
            }
        }
    }
};

void onScanComplete(BLEScanResults results)
{
    // do nothing
}

void setupTPMS()
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void scanTPMS()
{
    if (millis() - last_scan_time < scanTime * 1000)
    {
        return;
    }
    pBLEScan->start(scanTime, onScanComplete, false);
    pBLEScan->clearResults();
    last_scan_time = millis();
}

bool TPMSFrameToCanFrame(const tpms_t &tpms_data, log_frame_t &tpms_frame)
{
    tpms_frame.header.id = TPMS_TPMS_FRAME_ID;
    tpms_frame.header.dlc = TPMS_TPMS_LENGTH;
    tpms_frame.header.ide = TPMS_TPMS_IS_EXTENDED;

    tpms_tpms_t tpms = {
        .id = tpms_tpms_id_encode(tpms_data.numeric_id),
        .pressure = tpms_tpms_pressure_encode(tpms_data.pressure),
        .temperature = tpms_tpms_temperature_encode(tpms_data.temperature)};
    memset(tpms_frame.data, 0, tpms_frame.header.dlc);
    tpms_tpms_pack(tpms_frame.data, &tpms, tpms_frame.header.dlc);

    return true;
}
