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

bool waitForGPSFix(uint32_t timeout_ms)
{
    uint32_t start = millis();

    while (millis() - start < timeout_ms)
    {
        while (GPS.available())
        {
            GPS.read();
        }

        if (GPS.newNMEAreceived())
        {
            GPS.parse(GPS.lastNMEA());
        }

        if (GPS.fix)
        {
            return true;
        }

        // print dot
        // static uint32_t lastDot = 0;
        // if (millis() - lastDot > 1000)
        // {
        //     Serial.print('.');
        //     lastDot = millis();
        // }
    }
    return false;
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
