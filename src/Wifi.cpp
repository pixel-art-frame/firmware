#include "WiFi.hpp"
#include "Global.h"
#include "MatrixGif.hpp"
#include <WiFi.h>
#include <DNSServer.h>

#define CONN_GIF "/wifi_connecting.gif"

const byte DNS_PORT = 53;
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);

unsigned long connect_start = 0;

/**
 * If there are AP credentials stored try to connect
 * Otherwise create an AP
 * 
 */
void setupWifi()
{
    if (config.ssid != "")
    {
        connect();
        return;
    }

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
    WiFi.setHostname(String("Pixel Art Frame").c_str());
    WiFi.begin(config.ssid.c_str(), config.pass.c_str());

    target_state = CONNECT_WIFI;
    connect_start = millis();
}

void connecting()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        target_state = PLAYING_ART;
        return;
    }

    if (millis() - connect_start > 10000)
    {
        createAP();
    }
}

void handleDns()
{
    dnsServer.processNextRequest();
}