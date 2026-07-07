#include "gps.h"

Adafruit_GPS GPS(&GPSSerial);

bool setupGPS()
{
    // begin at 9600 baud
    GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    if (!GPS.begin(9600))
    {
        return false;
    }

    // change baud to 115200
    GPS.sendCommand(PMTK_SET_BAUD_115200);
    delay(100);
    GPSSerial.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);
    if (!GPS.begin(115200))
    {
        return false;
    }
    delay(100);

    // config
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);

    return true;
}

bool readGPS()
{
    while (GPS.available())
        GPS.read();
    return GPS.newNMEAreceived() && GPS.parse(GPS.lastNMEA());
}

void syncTimeFromGPS()
{
    struct tm t = {};
    t.tm_year = GPS.year + 100;
    t.tm_mon = GPS.month - 1;
    t.tm_mday = GPS.day;
    t.tm_hour = GPS.hour;
    t.tm_min = GPS.minute;
    t.tm_sec = GPS.seconds;
    t.tm_isdst = 0;

    // set system time to UTC
    setenv("TZ", "UTC0", 1);
    tzset();
    time_t utc = mktime(&t);

    // set system time to local time (assumed EST)
    setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
    tzset();

    struct timeval tv = {.tv_sec = utc, .tv_usec = 0};
    settimeofday(&tv, nullptr);
}

bool GPSFrameToCanFrame(twai_frame_t frame[4])
{
    frame[0].header.id = CANMOD_GPS_GNSS_POSITION_FRAME_ID;
    frame[0].header.ide = CANMOD_GPS_GNSS_POSITION_IS_EXTENDED;
    frame[0].header.dlc = CANMOD_GPS_GNSS_POSITION_LENGTH;
    canmod_gps_gnss_position_t gps_position = {
        .position_valid = canmod_gps_gnss_position_position_valid_encode(GPS.fix),
        .latitude = canmod_gps_gnss_position_latitude_encode(GPS.latitudeDegrees),
        .longitude = canmod_gps_gnss_position_longitude_encode(GPS.longitudeDegrees),
        .position_accuracy = canmod_gps_gnss_position_position_accuracy_encode(GPS.fixquality)
    };
    if (frame[0].buffer == nullptr || frame[0].buffer_len < frame[0].header.dlc)
    {
        return false;
    }
    memset(frame[0].buffer, 0, frame[0].buffer_len);
    canmod_gps_gnss_position_pack(frame[0].buffer, &gps_position, frame[0].header.dlc);

    frame[1].header.id = CANMOD_GPS_GNSS_ALTITUDE_FRAME_ID;
    frame[1].header.ide = CANMOD_GPS_GNSS_ALTITUDE_IS_EXTENDED;
    frame[1].header.dlc = CANMOD_GPS_GNSS_ALTITUDE_LENGTH;
    canmod_gps_gnss_altitude_t gps_altitude = {
        .altitude_valid = canmod_gps_gnss_altitude_altitude_valid_encode(GPS.fixquality),
        .altitude = canmod_gps_gnss_altitude_altitude_encode(GPS.altitude),
        .altitude_accuracy = 0 // TODO: implement altitude accuracy
    };
    if (frame[1].buffer == nullptr || frame[1].buffer_len < frame[1].header.dlc)
    {
        return false;
    }
    memset(frame[1].buffer, 0, frame[1].buffer_len);
    canmod_gps_gnss_altitude_pack(frame[1].buffer, &gps_altitude, frame[1].header.dlc);

    frame[2].header.id = CANMOD_GPS_GNSS_SPEED_FRAME_ID;
    frame[2].header.ide = CANMOD_GPS_GNSS_SPEED_IS_EXTENDED;
    frame[2].header.dlc = CANMOD_GPS_GNSS_SPEED_LENGTH;
    canmod_gps_gnss_speed_t gps_speed = {
        .speed_valid = canmod_gps_gnss_speed_speed_valid_encode(GPS.fixquality),
        .speed = canmod_gps_gnss_speed_speed_encode(GPS.speed),
        .speed_accuracy = 0 // TODO: implement speed accuracy
    };
    if (frame[2].buffer == nullptr || frame[2].buffer_len < frame[2].header.dlc)
    {
        return false;
    }
    memset(frame[2].buffer, 0, frame[2].buffer_len);
    canmod_gps_gnss_speed_pack(frame[2].buffer, &gps_speed, frame[2].header.dlc);

    frame[3].header.id = CANMOD_GPS_GNSS_STATUS_FRAME_ID;
    frame[3].header.ide = CANMOD_GPS_GNSS_STATUS_IS_EXTENDED;
    frame[3].header.dlc = CANMOD_GPS_GNSS_STATUS_LENGTH;
    canmod_gps_gnss_status_t gps_status = {
        .fix_type = GPS.fixquality,
        .satellites = GPS.satellites,
    };
    if (frame[3].buffer == nullptr || frame[3].buffer_len < frame[3].header.dlc)
    {
        return false;
    }
    memset(frame[3].buffer, 0, frame[3].buffer_len);
    canmod_gps_gnss_status_pack(frame[3].buffer, &gps_status, frame[3].header.dlc);

    return true;
}
