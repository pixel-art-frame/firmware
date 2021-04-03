#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Global.h"

class ConfigApi
{
public:
    void handleWifiConfig(AsyncWebServerRequest *request);
    void handleScanWifi(AsyncWebServerRequest *request);

private:
String translateEncryptionType(wifi_auth_mode_t encryptionType);
};