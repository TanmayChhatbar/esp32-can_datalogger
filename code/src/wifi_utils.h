#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SD.h>
#include "credentials.h"

extern WebServer server;
extern const char *host;
extern File uploadFile;

bool wifiEnable();
void returnOK();
void returnFail(String msg);
bool loadFromSdCard(String path);
void handleFileUpload();
void deleteRecursive(String path);
void handleDelete();
void handleCreate();
void printDirectory();
void handleNotFound();
