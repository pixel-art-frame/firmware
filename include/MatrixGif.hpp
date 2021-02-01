/**
 *  Draw AnimatedGIF from SPIFFS/SD to an RGB LED matrix using the ESP32 I2S DMA library
 *  Credits: https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/tree/master/examples/AnimatedGIFPanel
 */
#ifndef _MATRIX_GIF_
#define _MATRIX_GIF_

#include <AnimatedGIF.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>

void GIFDraw(GIFDRAW *pDraw);

void * GIFOpenFile(const char *fname, int32_t *pSize);

void GIFCloseFile(void *pHandle);

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition);

void ShowGIF(char *name, bool fromSpiffs = false);

void InitMatrixGif(VirtualMatrixPanel *panel);

#endif