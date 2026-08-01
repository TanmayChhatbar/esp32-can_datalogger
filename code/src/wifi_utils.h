#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SD.h>
#include "credentials.h"
#include "defs.h"

extern WebServer server;
extern const char *host;
extern File uploadFile;

bool wifiEnable();
bool wifiEnableAP();
void returnOK();
void returnFail(String msg);
bool loadFromSdCard(String path);
void handleNotFound();
bool wifiDisable();
