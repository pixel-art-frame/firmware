#include "Global.h"

Text text;

void showText(Text t)
{
    if (t.scroll)
    {
        scroll(t.text, t.speed, t.color, t.size, t.y);
        return;
    }

    println(t.text, t.color, t.size, t.x, t.y, t.wrap, t.clearScreen, t.delay);
}

void println(String text, uint16_t color = DEFAULT_TEXT_COLOR, uint8_t size = 1, int16_t x = 4, int16_t y = 4, bool wrap = true, bool clearScreen = true, int d = 4000)
{
    frame_state = SHOW_TEXT;

    if (clearScreen)  virtualDisp.clearScreen();

    virtualDisp.setTextWrap(wrap);
    virtualDisp.setCursor(x, y);
    virtualDisp.setTextColor(color);
    virtualDisp.setTextSize(size);
    virtualDisp.println(text);

    delay(d); // TODO: Remove delay
    frame_state = PLAYING_ART;
}

void scroll(String text, int speed = 50, uint16_t color = DEFAULT_TEXT_COLOR, uint8_t size = 1, int16_t y = 4)
{
    frame_state = SHOW_TEXT;

    uint16_t text_length = text.length();

    virtualDisp.setTextWrap(false);
    virtualDisp.setTextSize(size);
    virtualDisp.setRotation(0);
    virtualDisp.setTextColor(color);
    virtualDisp.clearScreen();

    for (int xpos = MATRIX_WIDTH; xpos > -(MATRIX_WIDTH + text_length * (size * 6)); xpos--)
    {
        virtualDisp.setTextColor(color);
        virtualDisp.clearScreen();
        virtualDisp.setCursor(xpos, y);
        virtualDisp.println(text);
        delay(50); // TODO: Remove delay
        yield();

        // This might smooth the transition a bit if we go slow
        // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
        // display.setCursor(xpos-1,ypos);
        // display.println(text);

        //delay(scroll_delay/5);
        //yield();
    }

    frame_state = PLAYING_ART;
}