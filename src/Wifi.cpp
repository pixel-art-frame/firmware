#include "WiFi.hpp"
#include "Global.h"
#include <WiFi.h>
#include <DNSServer.h>

const byte DNS_PORT = 53;
DNSServer dnsServer;

IPAddress apIP(192, 168, 4, 1);

/**
 * If there are AP credentials stored try to connect
 * Otherwise create an AP
 * 
 */
void setupWifi()
{
    Serial.println("Setting up WIFI:");

    if (config.ssid != "")
    {
        Serial.println("Connectring to WIFI");
        connect();
        return;
    }

    Serial.println("No SSID saved, creating AP");

    createAP();
}

void createAP()
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("Pixel Art Frame", "PixelArt");
    dnsServer.start(DNS_PORT, "*", apIP);
    config.wifiMode = WIFI_AP_STA;
}

void connect()
{
    WiFi.begin(config.ssid.c_str(), config.pass.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
}

void handleDns()
{
    dnsServer.processNextRequest();
}