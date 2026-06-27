#include "sd_logger.h"

bool writeCanFrameToSD(const CanFrame &frame, unsigned long long timestamp, const uint8_t bus, File &file)
{
    String can_data_str = CanFrameToString(frame, timestamp, bus);
    file.println(can_data_str);
    return true;
}

bool writeCANHeaderToSD(File &file)
{
    String header = "Timestamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8";
    file.println(header);
    return true;
}
