#ifndef _GLOBAL_
#define _GLOBAL_

#include <FS.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include "Configuration.hpp"

typedef enum {
    OFF = 0, // LED matrix is off
    PLAYING_ART, // Looping trough art
    CONNECT_WIFI, // Connecting to WiFi
    ADJ_BRIGHTNESS // Adjusting brightness

} frame_status_t;

extern frame_status_t frame_state;
extern unsigned long lastStateChange;

extern Config config;

extern MatrixPanel_I2S_DMA dma_display;

extern bool interruptGif, gifsLoaded;
extern std::vector<String> gifs;
extern bool loadGifFromSpiffs;

extern int brightness;
extern bool autoPlay;

#endif