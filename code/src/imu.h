#pragma once
#include "pindef.h"
#include "can.h"
#include "can_imu_14bit.h"
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>

struct imu_event_t
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t mag;
    sensors_event_t temp;
};

extern Adafruit_ICM20948 icm;
bool setupIMU();
bool readIMU(imu_event_t &imu_event);
bool IMUFrameToCanFrame(const imu_event_t &imu_event, log_frame_t imu_frames[2]);
