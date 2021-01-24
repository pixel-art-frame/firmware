#ifndef _WEB_
#define _WEB_

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include "Webpage.h"

void initServer();
void configureWebServer();
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

String humanReadableSize(const size_t bytes);
String listFiles();
void handleBrightness(AsyncWebServerRequest *request);

#endif

