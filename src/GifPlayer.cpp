#include <list>
#include <iterator>
#include <SD.h>
#include "Global.h"
#include "MatrixGif.hpp"
#include "GifLoader.hpp"

bool gifsLoaded = false,
     autoPlay = true;

unsigned long gifStart = 0;
int minPlaytime = 4000;
char *gifDir = "/gifs";
String currentGif;
int currentGifIndex = 0;

std::vector<String> gifs; // TODO: Store history here

void loadGifs()
{
    return;
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

void nextGif()
{
    currentGif = getNextGif();
    
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
    if (currentGif.length() == 0) {
        nextGif(); // TODO: Handle empty queue
    }

    while (!currentGif.endsWith(".gif")) {
        nextGif();
    }

    if (!gifPlaying && (autoPlay && millis() - gifStart > minPlaytime))
    {
        nextGif();
    }

    char gif[currentGif.length()+1];
    currentGif.toCharArray(gif, sizeof(gif));

    ShowGIF(gif);
}
