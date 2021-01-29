#ifndef _CONFIGURATION_
#define _CONFIGURATION_

#define CONFIG_FILENAME "/config.json"
#define CONFIG_SIZE 32

struct Config
{
  int brightness;
  bool autoPlay;
} ;


void loadSettings();
void saveSettings(const Config &config);

#endif