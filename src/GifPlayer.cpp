#include <list>
#include <iterator>
#include <SD.h>
#include "Global.h"
#include "MatrixGif.hpp"

bool gifsLoaded = false,
     autoPlay = true;

unsigned long gifStart = 0;
int minPlaytime = 4000;
char *gifDir = "/gifs";
int currentGifIndex = 0;

File currentGif;

std::vector<String> gifs;

void loadGifs()
{
    gifs.clear();
    File root = SD.open(gifDir);

    if (!root)
    {
        Serial.println("Failed to open gif dir");
        return;
    }

    File currentGif = root.openNextFile();

    while (currentGif)
    {
        if (currentGif.isDirectory())
        {
            currentGif.close();
            continue;
        }

        Serial.println("Loaded " + String(currentGif.name()));

        gifs.push_back(String((char *)currentGif.name()));

        currentGif.close();
        currentGif = root.openNextFile();
    }

    gifsLoaded = true;
}

char *getCurrentGif()
{
    return (char *)currentGif.name();
}

void nextGif()
{
    
    //currentGif.ne
    
    gifStart = millis();
    interruptGif = true;
}

void prevGif()
{
    gifStart = millis();

    if (currentGifIndex > 0)
    {
        currentGifIndex--;
    }
    else
    {
        currentGifIndex = gifs.size() - 1;
    }
    interruptGif = true;
}

void setGif(int index)
{
    if (index < 0 || index >= gifs.size())
    {
        return;
    }

    gifStart = millis();
    currentGifIndex = index;
    interruptGif = true;
}

void handleGif()
{
    if (!gifPlaying && (autoPlay && millis() - gifStart > minPlaytime))
    {
        nextGif();
    }

    ShowGIF(getCurrentGif());
}
