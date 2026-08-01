/*
  SDWebServer - Example WebServer with SD Card backend for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Have a FAT Formatted SD Card connected to the SPI port of the ESP8266
  The web root is the SD Card root folder
  File extensions with more than 3 characters are not supported by the SD Library
  File Names longer than 8 characters will be truncated by the SD library, so keep filenames shorter
  index.htm is the default index (works on subfolders as well)

  upload the contents of SdRoot to the root of the SDcard and access the editor by going to http://esp8266sd.local/edit
  To retrieve the contents of SDcard, visit http://esp32sd.local/list?dir=/
      dir is the argument that needs to be passed to the function PrintDirectory via HTTP Get request.

*/
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "defs.h"
#include "wifi_utils.h"

void setup(void)
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.print("\n");
    
    wifiEnable();

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS_IMU);
    if (SD.begin(SPI_CS_SD, SPI, 40000000))
    {
        Serial.println("SD Card initialized");
    }
    else
    {
        Serial.println("SD Card init failed");
        while (1)
        {
            delay(500);
        }
    }
}

void loop(void)
{
    server.handleClient();
    delay(2); // allow the cpu to switch to other tasks
}
