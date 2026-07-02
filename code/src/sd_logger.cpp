#include "sd_logger.h"

uint64_t getEpochMicroseconds()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    return ((int64_t)tv.tv_sec * 1000000LL) + tv.tv_usec;
}

bool writeCanFrameToSD(const twai_message_t &frame, unsigned long long timestamp, const uint8_t bus, File &file)
{
    String can_data_str = CanFrameToString(frame, timestamp, bus);
    file.println(can_data_str);
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
