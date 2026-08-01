#include "gps.h"

Adafruit_GPS GPS(&GPSSerial);

bool setupGPS()
{
    // begin at 9600 baud
    GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    if (GPS.begin(9600))
    {
        // change baud to 115200
        delay(50);
        GPS.sendCommand(PMTK_SET_BAUD_115200);
        delay(200);
    }
    GPSSerial.end();
    delay(5);
    GPSSerial.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);
    delay(5);
    GPS.begin(115200);
    delay(40);

    // config
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    delay(20);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
    delay(20);
    GPS.sendCommand(PGCMD_ANTENNA);
    delay(20);

    unsigned long start = millis();
    while (millis() - start < 10000)
    {
        unsigned int counter = 0;
        while (GPS.available())
        {
            char c = GPS.read();
            counter++;
            if (counter > 10)
            {
                break;
            }
        }
        if (GPS.newNMEAreceived())
        {
            if (GPS.parse(GPS.lastNMEA()))
            {
                return true;
            }
        }
    }
    return false;
}

bool readGPS()
{
    while (GPS.available())
    {
        char c = GPS.read();
        // Serial.write(c);
    }
    if (GPS.newNMEAreceived())
    {
        return GPS.parse(GPS.lastNMEA());
    }
    return false;
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

bool GPSFrameToCanFrame(log_frame_t log_frame[4])
{
    log_frame[0].header.id = CANMOD_GPS_GNSS_POSITION_FRAME_ID;
    log_frame[0].header.ide = CANMOD_GPS_GNSS_POSITION_IS_EXTENDED;
    log_frame[0].header.dlc = CANMOD_GPS_GNSS_POSITION_LENGTH;
    canmod_gps_gnss_position_t gps_position = {
        .position_valid = canmod_gps_gnss_position_position_valid_encode(GPS.fix),
        .latitude = canmod_gps_gnss_position_latitude_encode(GPS.latitudeDegrees),
        .longitude = canmod_gps_gnss_position_longitude_encode(GPS.longitudeDegrees),
        .position_accuracy = canmod_gps_gnss_position_position_accuracy_encode(GPS.fixquality)};
    memset(log_frame[0].data, 0, log_frame[0].header.dlc);
    canmod_gps_gnss_position_pack(log_frame[0].data, &gps_position, log_frame[0].header.dlc);

    log_frame[1].header.id = CANMOD_GPS_GNSS_ALTITUDE_FRAME_ID;
    log_frame[1].header.ide = CANMOD_GPS_GNSS_ALTITUDE_IS_EXTENDED;
    log_frame[1].header.dlc = CANMOD_GPS_GNSS_ALTITUDE_LENGTH;
    canmod_gps_gnss_altitude_t gps_altitude = {
        .altitude_valid = canmod_gps_gnss_altitude_altitude_valid_encode(GPS.fixquality),
        .altitude = canmod_gps_gnss_altitude_altitude_encode(GPS.altitude),
        .altitude_accuracy = 0 // TODO: implement altitude accuracy
    };
    memset(log_frame[1].data, 0, log_frame[1].header.dlc);
    canmod_gps_gnss_altitude_pack(log_frame[1].data, &gps_altitude, log_frame[1].header.dlc);

    log_frame[2].header.id = CANMOD_GPS_GNSS_SPEED_FRAME_ID;
    log_frame[2].header.ide = CANMOD_GPS_GNSS_SPEED_IS_EXTENDED;
    log_frame[2].header.dlc = CANMOD_GPS_GNSS_SPEED_LENGTH;
    canmod_gps_gnss_speed_t gps_speed = {
        .speed_valid = canmod_gps_gnss_speed_speed_valid_encode(GPS.fixquality),
        .speed = canmod_gps_gnss_speed_speed_encode(GPS.speed),
        .speed_accuracy = 0 // TODO: implement speed accuracy
    };
    memset(log_frame[2].data, 0, log_frame[2].header.dlc);
    canmod_gps_gnss_speed_pack(log_frame[2].data, &gps_speed, log_frame[2].header.dlc);

    log_frame[3].header.id = CANMOD_GPS_GNSS_STATUS_FRAME_ID;
    log_frame[3].header.ide = CANMOD_GPS_GNSS_STATUS_IS_EXTENDED;
    log_frame[3].header.dlc = CANMOD_GPS_GNSS_STATUS_LENGTH;
    canmod_gps_gnss_status_t gps_status = {
        .fix_type = GPS.fixquality,
        .satellites = GPS.satellites,
    };
    memset(log_frame[3].data, 0, log_frame[3].header.dlc);
    canmod_gps_gnss_status_pack(log_frame[3].data, &gps_status, log_frame[3].header.dlc);

    return true;
}
