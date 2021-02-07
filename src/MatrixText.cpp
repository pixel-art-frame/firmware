#include "Global.h"

Text text;

unsigned long show_text_start = 0;
unsigned long scroll_last_time = 0;

bool scrolling = false,
     scroll_finished = false;

int text_delay = 2000,
    scroll_speed = 50,
    scroll_x = 0,
    scroll_y = 0,
    scroll_text_length = 0,
    scroll_text_size = 1;

void handleText()
{
    if ((!scrolling && millis() - show_text_start > text_delay) || scroll_finished)
    {
        target_state = PLAYING_ART;
    }

    if (scrolling)
    {
        updateScroll();
    }
}

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
    target_state = SHOW_TEXT;
    show_text_start = millis();

    if (clearScreen)
        virtualDisp.clearScreen();

    virtualDisp.setTextWrap(wrap);
    virtualDisp.setCursor(x, y);
    virtualDisp.setTextColor(color);
    virtualDisp.setTextSize(size);
    virtualDisp.println(text);

    text_delay = d;
}

void scroll(String text, int speed = 50, uint16_t color = DEFAULT_TEXT_COLOR, uint8_t size = 1, int16_t y = 4)
{
    target_state = SHOW_TEXT;
    show_text_start = millis();

    scroll_text_length = text.length();
    scroll_text_size = size;
    scroll_speed = speed;
    scroll_x = MATRIX_WIDTH/2;
    scroll_y = y;
    scroll_finished = false;
    scrolling = true;

    virtualDisp.setTextWrap(false);
    virtualDisp.setTextSize(size);
    virtualDisp.setRotation(0);
    virtualDisp.setTextColor(color);
    virtualDisp.clearScreen();
}

void updateScroll()
{
    scroll_finished = scroll_x < -((MATRIX_WIDTH/2) + scroll_text_length * (scroll_text_size * 6));

    if (scroll_finished)
    {
        return;
    }

    if (millis() - scroll_last_time < scroll_speed)
    {
        return;
    }

    virtualDisp.setTextColor(text.color);
    virtualDisp.clearScreen();
    virtualDisp.setCursor(scroll_x, scroll_y);
    virtualDisp.println(text.text);

    // This might smooth the transition a bit if we go slow
    // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
    // display.setCursor(xpos-1,ypos);
    // display.println(text);

    //delay(scroll_delay/5);
    //yield();

    scroll_x--;
    scroll_last_time = millis();
}