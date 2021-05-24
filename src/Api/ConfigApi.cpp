#include "Api/ConfigApi.hpp"

void ConfigApi::handleWifiConfig(AsyncWebServerRequest *request)
{
    if (!request->hasParam("ssid") || !request->hasParam("pass"))
    {
        request->send(400, "text/plain", "Missing parameter(s): ssid, value");
        return;
    }

    config.ssid = request->getParam("ssid")->value();
    config.pass = request->getParam("pass")->value();
    saveSettings();

    request->send(200, "text/plain", "Restart to apply changes");
}

String ConfigApi::translateEncryptionType(wifi_auth_mode_t encryptionType)
{
    switch (encryptionType)
    {
    case (0):
        return "Open";
    case (1):
        return "WEP";
    case (2):
        return "WPA_PSK";
    case (3):
        return "WPA2_PSK";
    case (4):
        return "WPA_WPA2_PSK";
    case (5):
        return "WPA2_ENTERPRISE";
    default:
        return "UNKOWN";
    }
}

void ConfigApi::handleScanWifi(AsyncWebServerRequest *request)
{
    int16_t n = WiFi.scanNetworks();

    String jsonResponse = "[";

    for (int i = 0; i < n; i++)
    {
        jsonResponse += "{\"ssid\": \"" + WiFi.SSID(i) + "\", \"rssi\": \"" + WiFi.RSSI(i) + "\", \"auth\": \"" + translateEncryptionType(WiFi.encryptionType(i)) + "\"}";

        if (i < (n - 1))
            jsonResponse += ",";
    }

    jsonResponse += "]";

    request->send(200, "application/json", jsonResponse);
}