#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "tpms.h"
#include "can.h"

struct tpms_t
{
    String id;
    String name;
    unsigned int numeric_id;
    float pressure;    // psi
    float temperature; // deg C
};
#define TPMS_COUNT 2
extern tpms_t tpms[TPMS_COUNT];
extern BLEScan *pBLEScan;

void setupTPMS();
void scanTPMS();
bool TPMSFrameToCanFrame(const tpms_t &tpms_data, log_frame_t tpms_frame);
