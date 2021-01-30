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
    
}


void handleDns()
{
   dnsServer.processNextRequest();

}