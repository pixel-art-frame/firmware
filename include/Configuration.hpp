#ifndef _CONFIGURATION_
#define _CONFIGURATION_

#include <WiFi.h>
#include "GifLoader.hpp"

#define CONFIG_FILENAME "/config.json"
#define CONFIG_SIZE 512

struct Config
{
  int brightness;
  bool autoPlay;

  wifi_mode_t wifiMode;
  String ssid;
  String pass;

  bool enableTime;
  int timeInterval;
  int timeShowSeconds;
  int timeOffset;
  String ntpServer;

  load_strategy_t loadStrategy;
};

void loadSettings();
void saveSettings();

#endif