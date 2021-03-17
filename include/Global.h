#ifndef _GLOBAL_
#define _GLOBAL_

#include <FS.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include "Configuration.hpp"
#include "MatrixText.hpp"

typedef enum {
    OFF = 0, // LED matrix is off
    PLAYING_ART, // Looping trough art
    SHOW_TEXT,
    SHOW_TIME,
    STARTUP,

    CONNECT_WIFI, // Connecting to WiFi
    ADJ_BRIGHTNESS, // Adjusting brightness

    OTA_UPDATE

} frame_status_t;

extern bool sd_ready;

extern frame_status_t target_state;
extern unsigned long lastStateChange;

extern Config config;

extern MatrixPanel_I2S_DMA dma_display;
extern VirtualMatrixPanel virtualDisp;

extern bool interruptGif, gifsLoaded;
extern std::vector<String> gifs;
extern bool loadGifFromSpiffs;

extern Text text;

extern int brightness;
extern bool autoPlay;
extern bool gifPlaying;
extern bool queue_populate_requred;

#endif