#include "imu.h"

Adafruit_ICM20948 icm;
#define RAD2DEG(x) (x*(180.0 / M_PI))

bool setupIMU()
{
    if (!icm.begin_SPI(SPI_CS_IMU, &SPI, 0))
    {
        return false;
    }

    // set range
    icm.setAccelRange(ICM20948_ACCEL_RANGE_8_G);
    icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);

    // frequencies
    icm.setAccelRateDivisor(10);
    icm.setGyroRateDivisor(10);
    icm.setMagDataRate(AK09916_MAG_DATARATE_50_HZ);

    return true;
}

bool readIMU(imu_event_t &imu_event)
{
    return icm.getEvent(&imu_event.accel, &imu_event.gyro, &imu_event.temp, &imu_event.mag);

    // Serial.print("\t\tTemperature ");
    // Serial.print(temp.temperature);
    // Serial.println(" deg C");
    // Serial.print("\t\tAccel X: ");
    // Serial.print(accel.acceleration.x);
    // Serial.print(" \tY: ");
    // Serial.print(accel.acceleration.y);
    // Serial.print(" \tZ: ");
    // Serial.print(accel.acceleration.z);
    // Serial.println(" m/s^2 ");
    // Serial.print("\t\tMag X: ");
    // Serial.print(mag.magnetic.x);
    // Serial.print(" \tY: ");
    // Serial.print(mag.magnetic.y);
    // Serial.print(" \tZ: ");
    // Serial.print(mag.magnetic.z);
    // Serial.println(" uT");
    // Serial.print("\t\tGyro X: ");
    // Serial.print(gyro.gyro.x);
    // Serial.print(" \tY: ");
    // Serial.print(gyro.gyro.y);
    // Serial.print(" \tZ: ");
    // Serial.print(gyro.gyro.z);
    // Serial.println(" radians/s ");
    // Serial.println();
    // return true;
}

bool IMUFrameToCanFrame(const imu_event_t &imu_event, log_frame_t imu_frames[2])
{
    imu_frames[0].header.id = CAN_IMU_14BIT_IMU_1_FRAME_ID;
    imu_frames[0].header.ide = CAN_IMU_14BIT_IMU_1_IS_EXTENDED;
    imu_frames[0].header.dlc = CAN_IMU_14BIT_IMU_1_LENGTH;
    can_imu_14bit_imu_1_t imu_1 = {
        .acceleration_x = can_imu_14bit_imu_1_acceleration_x_encode(imu_event.accel.acceleration.x / 9.81f),
        .acceleration_y = can_imu_14bit_imu_1_acceleration_y_encode(imu_event.accel.acceleration.y / 9.81f),
        .acceleration_z = can_imu_14bit_imu_1_acceleration_z_encode(imu_event.accel.acceleration.z / 9.81f),
        .angular_rate_x = can_imu_14bit_imu_1_angular_rate_x_encode(RAD2DEG(imu_event.gyro.gyro.x)),
    };
    memset(imu_frames[0].data, 0, imu_frames[0].header.dlc);
    can_imu_14bit_imu_1_pack(imu_frames[0].data, &imu_1, imu_frames[0].header.dlc);

    imu_frames[1].header.id = CAN_IMU_14BIT_IMU_2_FRAME_ID;
    imu_frames[1].header.ide = CAN_IMU_14BIT_IMU_2_IS_EXTENDED;
    imu_frames[1].header.dlc = CAN_IMU_14BIT_IMU_2_LENGTH;
    can_imu_14bit_imu_2_t imu_2 = {
        .angular_rate_y = can_imu_14bit_imu_2_angular_rate_y_encode(RAD2DEG(imu_event.gyro.gyro.y)),
        .angular_rate_z = can_imu_14bit_imu_2_angular_rate_z_encode(RAD2DEG(imu_event.gyro.gyro.z)),
        .mag_x = can_imu_14bit_imu_2_mag_x_encode(imu_event.mag.magnetic.x),
        .mag_y = can_imu_14bit_imu_2_mag_y_encode(imu_event.mag.magnetic.y),
        .mag_z = can_imu_14bit_imu_2_mag_z_encode(imu_event.mag.magnetic.z),
    };
    memset(imu_frames[1].data, 0, imu_frames[1].header.dlc);
    can_imu_14bit_imu_2_pack(imu_frames[1].data, &imu_2, imu_frames[1].header.dlc);

    return true;
}
