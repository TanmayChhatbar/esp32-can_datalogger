#include "sd_logger.h"

uint64_t getEpochMicroseconds()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    return ((int64_t)tv.tv_sec * 1000000LL) + tv.tv_usec;
}

bool writeCanFrameToSD(const log_frame_t &log_frame, File &file)
{
    // convert to string
    char buf[70];
    CanFrameToString(log_frame, buf, sizeof(buf));

    // write to file
    file.println(buf);

    // print to serial for printing
#if SERIAL_PRINT_CAN
#if !SERIAL_GPS
    if (log_frame.header.id == CANMOD_GPS_GNSS_STATUS_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_TIME_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_POSITION_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_ALTITUDE_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_ATTITUDE_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_ODO_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_SPEED_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_GEOFENCE_FRAME_ID ||
        log_frame.header.id == CANMOD_GPS_GNSS_IMU_FRAME_ID)
    {
        return true;
    }
#endif
#if !SERIAL_IMU
    if (log_frame.header.id == CAN_IMU_14BIT_IMU_1_FRAME_ID ||
        log_frame.header.id == CAN_IMU_14BIT_IMU_2_FRAME_ID)
    {
        return true;
    }
#endif
#if !SERIAL_CAN0
    if (log_frame.bus_id == 0)
    {
        return true;
    }
#endif
#if !SERIAL_CAN1
    if (log_frame.bus_id == 1)
    {
        return true;
    }
#endif
    Serial.println(buf);
#endif
    return true;
}

bool writeCANHeaderToSD(File &file)
{
    return file.println("Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8") > 0;
}

const char *getFilenameForCurrentTime()
{
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    static char filename[36];
    strftime(filename, sizeof(filename), "/YamahaR7_%Y-%m-%d_%H-%M-%S.log", timeinfo);
    return filename;
}

String loggingModeToString(logging_mode_t logging_mode)
{
    switch (logging_mode)
    {
    case LOGGING_MODE_WAITGPS:
        return "LOGGING_MODE_WAITGPS";
    case LOGGING_MODE_INACTIVE:
        return "LOGGING_MODE_INACTIVE";
    case LOGGING_MODE_STOP:
        return "LOGGING_MODE_STOP";
    case LOGGING_MODE_START:
        return "LOGGING_MODE_START";
    case LOGGING_MODE_ACTIVE:
        return "LOGGING_MODE_ACTIVE";
    case LOGGING_MODE_WIFI_START:
        return "LOGGING_MODE_WIFI_START";
    case LOGGING_MODE_WIFI_ACTIVE:
        return "LOGGING_MODE_WIFI_ACTIVE";
    case LOGGING_MODE_WIFI_STOP:
        return "LOGGING_MODE_WIFI_STOP";
    default:
        return "LOGGING_MODE_UNKNOWN";
    }
}
