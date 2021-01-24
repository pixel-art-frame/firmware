#ifndef _GLOBAL_
#define _GLOBAL_

#include <FS.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>

extern MatrixPanel_I2S_DMA dma_display;
extern bool interruptGif, gifsLoaded;
extern std::vector<String> gifs;

#endif