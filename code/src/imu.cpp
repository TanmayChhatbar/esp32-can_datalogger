#include "imu.h"

Adafruit_ICM20948 icm;

bool setupIMU()
{
    if (!icm.begin_SPI(SPI_CS_IMU, &SPI, 0))
    {
        return false;
    }

    // set range
    icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
    icm.setGyroRange(ICM20948_GYRO_RANGE_250_DPS);

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

bool IMUFrameToCanFrame(const imu_event_t &imu_event, log_frame_t &log_frame)
{
    log_frame.header.id = CANMOD_GPS_GNSS_IMU_FRAME_ID;
    log_frame.header.ide = CANMOD_GPS_GNSS_IMU_IS_EXTENDED;
    log_frame.header.dlc = CANMOD_GPS_GNSS_IMU_LENGTH;
    canmod_gps_gnss_imu_t gps_imu = {
        .imu_valid = canmod_gps_gnss_imu_imu_valid_encode(1.0f),
        .acceleration_x = canmod_gps_gnss_imu_acceleration_x_encode(imu_event.accel.acceleration.x),
        .acceleration_y = canmod_gps_gnss_imu_acceleration_y_encode(imu_event.accel.acceleration.y),
        .acceleration_z = canmod_gps_gnss_imu_acceleration_z_encode(imu_event.accel.acceleration.z),
        .angular_rate_x = canmod_gps_gnss_imu_angular_rate_x_encode(imu_event.gyro.gyro.x),
        .angular_rate_y = canmod_gps_gnss_imu_angular_rate_y_encode(imu_event.gyro.gyro.y),
        .angular_rate_z = canmod_gps_gnss_imu_angular_rate_z_encode(imu_event.gyro.gyro.z),
    };
    memset(log_frame.data, 0, log_frame.header.dlc);
    canmod_gps_gnss_imu_pack(log_frame.data, &gps_imu, log_frame.header.dlc);

    return true;
}
