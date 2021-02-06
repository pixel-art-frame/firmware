#ifndef _OTA_
#define _OTA_

#include <ArduinoOTA.h>
#include "Global.h"

#define OTA_MODE_TIMEOUT 120000

bool ota_ready = false,
    ota_started = false;

unsigned long ota_mode_start;

void setupOTA()
{
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "firmware";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
            println("Updating\n" + type, virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            ota_started = true;
        })
        .onEnd([]() {
            println("Finished!\nRestarting", virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            delay(4000);
            ESP.restart();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            int percent = (progress / (total / 100));

            Serial.println("OTA progress: " + String(percent) + "%");
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            println("Error!" , virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 2000);
            if (error == OTA_AUTH_ERROR)
            {
                Serial.println("Auth Failed");
                println("Auth failed", virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            }
            else if (error == OTA_BEGIN_ERROR)
            {
                Serial.println("Begin Failed");
                println("Begin Failed", virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            }
            else if (error == OTA_CONNECT_ERROR)
            {
                Serial.println("Connect Failed");
                println("Connect Failed", virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            }
            else if (error == OTA_RECEIVE_ERROR)
            {
                Serial.println("Receive Failed");
                println("Receive Failed", virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            }
            else if (error == OTA_END_ERROR)
            {
                Serial.println("End Failed");
                println("End Failed", virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            }

            delay(4000);
            println("Restarting", virtualDisp.color565(0, 255, 0), 1, 4, 4, true, true, 0);
            delay(4000);
            ESP.restart();
        });

    ArduinoOTA.begin();
}

void handleOTA()
{
    if (!ota_ready)
    {
        println("Setting up OTA", virtualDisp.color565(0, 255, 0), 1, 0, 4, true, true, 0);

        ota_mode_start = millis();

        setupOTA();

        ota_ready = true;
        frame_state = OTA_UPDATE;
        interruptGif = true;
        println("Ready for OTA update", virtualDisp.color565(0, 255, 0), 1, 0, 4, true, true, 0);
    }

    if (!ota_started && millis() - ota_mode_start > OTA_MODE_TIMEOUT)
    {
        println("OTA\nTimeout\n\nRestarting", virtualDisp.color565(255, 0, 0), 1, 0, 4, true, true, 1000);
        delay(4000);
        ESP.restart();
        return;
    }

    ArduinoOTA.handle();
}

#endif