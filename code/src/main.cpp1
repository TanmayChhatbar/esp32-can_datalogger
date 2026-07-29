// GPS test
#include <Arduino.h>
#include <FastLED.h>
#include "defs.h"
#include "gps.h"

void setup()
{
    Serial.begin(921600);
    Serial.setTxTimeoutMs(0);

    // set up GPS
    Serial.println("GPS");
    delay(3000);
    while (!setupGPS())
    {
        Serial.println("\tFailed to initialize");
        delay(1000);
    }
    Serial.println("\tInitialized");

    // wait for fix
    Serial.println("GPS\n\tWaiting for GPS fix");
    unsigned long wait_gps_start_ms = millis();
    bool last_gps_fix_state = false;

    while (true)
    {
        if (millis() - wait_gps_start_ms > 1200)
        {
            if (readGPS())
            {
                // if (GPS.year > 25 && GPS.year < 30)
                // {
                    syncTimeFromGPS();
                    break;
                // }
                // else
                // {
                //     Serial.printf("\n\tGPS time not valid, waiting for next fix\n\t%d\n", GPS.year);
                //     wait_gps_start_ms = millis();
                // }
            }
        }

        bool gps_fix_var = digitalRead(GPS_FIX);
        if (gps_fix_var != last_gps_fix_state)
        {
            last_gps_fix_state = gps_fix_var;
            wait_gps_start_ms = millis();
        }
    }
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void loop()
{
    // if (readGPS())
    // {

    //     // Serial.printf("%d-%d-%d %d:%d:%d.%d\n", GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds);
    // }
    Serial.println(esp_timer_get_time());
    delay(1000);
}
