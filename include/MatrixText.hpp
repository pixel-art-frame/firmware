#ifndef _MATRIX_TEXT_
#define _MATRIX_TEXT_

#include "Global.h"

// 255 0 0
#define DEFAULT_TEXT_COLOR ((255 & 0xF8) << 8) | ((0 & 0xFC) << 3) | (0 >> 3)


struct Text
{
    String text;
    uint16_t color = DEFAULT_TEXT_COLOR;
    uint8_t size = 1;
    
    int16_t x = 4;
    int16_t y = 4;
    
    bool wrap = true;
    bool scroll = false;
    bool clearScreen = true;

    int speed = 50;
    int delay = 5000;
};

void handleText();

void showText(Text text);

void println(String text, uint16_t color, uint8_t size, int16_t x, int16_t y, bool wrap, bool clearScreen, int d);

void scroll(String text, int speed, uint16_t color, uint8_t size, int16_t y);
void updateScroll();

#endif