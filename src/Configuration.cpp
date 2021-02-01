#include "Configuration.hpp"
#include "Global.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define BRIGHTNESS_KEY "brightness"
#define AUTOPLAY_KEY "autoplay"
#define SSID_KEY "ssid"
#define PASS_KEY "pass"

void loadSettings()
{
  File configFile = SPIFFS.open(CONFIG_FILENAME, FILE_READ);

  StaticJsonDocument<CONFIG_SIZE> doc;

  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    Serial.println(F("Failed to read file, using default configuration"));
  }

  config.brightness = doc[BRIGHTNESS_KEY] | 20;
  config.autoPlay = doc[AUTOPLAY_KEY] | true;
  config.ssid = String(doc[SSID_KEY] | "");
  config.pass = String(doc[PASS_KEY] | "");

  configFile.close();
}

void saveSettings()
{
  StaticJsonDocument<CONFIG_SIZE> doc;

  doc[BRIGHTNESS_KEY] = config.brightness;
  doc[AUTOPLAY_KEY] = config.autoPlay;
  doc[SSID_KEY] = config.ssid;
  doc[PASS_KEY] = config.pass;

  if (SPIFFS.exists(CONFIG_FILENAME))
  {
    SPIFFS.remove(CONFIG_FILENAME);
  }

  File configFile = SPIFFS.open(CONFIG_FILENAME, FILE_WRITE);

  if (!configFile)
  {
    Serial.println(F("Failed to create file"));
    return;
  }

  if (serializeJson(doc, configFile) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }

  configFile.close();
}