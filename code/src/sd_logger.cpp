#include "sd_logger.h"

uint64_t getEpochMicroseconds()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    return ((int64_t)tv.tv_sec * 1000000LL) + tv.tv_usec;
}

bool writeCanFrameToSD(const twai_frame_t &frame, unsigned long long timestamp, const uint8_t bus, File &file)
{
    // convert to string
    // char can_data_str[64];
    // int n = CanFrameToCharArray(can_data_str, sizeof(can_data_str), frame, timestamp, bus);
    String can_data_str = CanFrameToString(frame, timestamp, bus);

    // write to file
    file.println(can_data_str);

    // print to serial for printing
    // Serial.println(can_data_str);
    
    return true;
}

bool writeCANHeaderToSD(File &file)
{
    String header = "Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8";
    file.println(header);
    return true;
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
