#include "Configuration.hpp"
#include "Global.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define BRIGHTNESS_KEY "brightness"
#define AUTOPLAY_KEY "autoplay"
#define SSID_KEY "ssid"
#define PASS_KEY "pass"
#define ENABLE_TIME_KEY "enable_time"
#define TIME_SECONDS_KEY "time_seconds"
#define TIME_INTERVAL_KEY "time_interval"
#define TIME_OFFSET_KEY "time_offset"
#define NTP_SERVER_KEY "ntp_server"

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

  config.enableTime = String(doc[ENABLE_TIME_KEY] | true);
  config.timeInterval = doc[TIME_INTERVAL_KEY] | 60000;
  config.timeShowSeconds = doc[TIME_SECONDS_KEY] | 5000;
  config.timeOffset = doc[TIME_OFFSET_KEY] | 0;

  configFile.close();
}

void saveSettings()
{
  StaticJsonDocument<CONFIG_SIZE> doc;

  doc[BRIGHTNESS_KEY] = config.brightness;
  doc[AUTOPLAY_KEY] = config.autoPlay;
  doc[SSID_KEY] = config.ssid;
  doc[PASS_KEY] = config.pass;

  doc[ENABLE_TIME_KEY] = config.enableTime;
  doc[TIME_INTERVAL_KEY] = config.timeInterval;
  doc[TIME_SECONDS_KEY] = config.timeShowSeconds;
  doc[TIME_OFFSET_KEY] = config.timeOffset;
  doc[NTP_SERVER_KEY] = config.ntpServer;

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