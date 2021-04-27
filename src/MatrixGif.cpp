#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include <AnimatedGIF.h>
#include <SD.h>
#include <SPIFFS.h>
#include "Global.h"

#define MAX_FILE_READ_COUNT 3

unsigned long start_tick = 0;
VirtualMatrixPanel *matrixGifPanel;
AnimatedGIF animGif;
File f;
int x_offset, y_offset;
bool interruptGif = false,
     loadGifFromSpiffs = false,
     gifPlaying = false;
int lastResult;
int8_t file_open_error_count = 0;

// Draw a line of image directly on the LED Matrix
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > MATRIX_WIDTH)
    iWidth = MATRIX_WIDTH;

  //Serial.println("pal565: " + String(*pDraw->pPalette) + " - pal888:" + String(*pDraw->pPalette24));

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line

  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }
  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + pDraw->iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < pDraw->iWidth)
    {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      }           // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        for (int xOffset = 0; xOffset < iCount; xOffset++)
        {
          matrixGifPanel->drawPixelRGB565(x + xOffset, y, usTemp[xOffset]);
        }
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          iCount++;
        else
          s--;
      }
      if (iCount)
      {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  }
  else // does not have transparency
  {
    s = pDraw->pPixels;

    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < pDraw->iWidth; x++)
    {
      matrixGifPanel->drawPixelRGB565(x, y, usPalette[*s++]);
    }
  }
} /* GIFDraw() */

void *GIFOpenFile(const char *fname, int32_t *pSize)
{
  if (loadGifFromSpiffs)
  {
    f = SPIFFS.open(fname);
  }
  else
  {
    if (!SD.exists(fname))
    {
      return NULL;
    }

    f = SD.open(fname);

    file_open_error_count = 0;
  }

  if (f)
  {
    *pSize = f.size();
    return (void *)&f;
  }

  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
    f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
  int32_t iBytesRead;
  iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  // Note: If you read a file all the way to the last byte, seek() stops working
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
  if (iBytesRead <= 0)
    return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{
  //  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  //  i = micros() - i;
  // Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

int LoadGIF(char *name)
{
  int result = animGif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw);

  x_offset = (MATRIX_WIDTH - animGif.getCanvasWidth()) / 2;
  if (x_offset < 0)
    x_offset = 0;

  y_offset = (MATRIX_HEIGHT - animGif.getCanvasHeight()) / 2;
  if (y_offset < 0)
    y_offset = 0;

  return result;
}

void ShowGIF(char *name, bool fromSpiffs = false)
{
  if (!gifPlaying)
  {
    loadGifFromSpiffs = fromSpiffs;

    if (!fromSpiffs && !SD.exists(name))
    {
      Serial.println("Attempted to show gif but it doesnt exist on SD: path:" + String(name));
      sd_state = UNMOUNTED;
      return;
    }

    if (fromSpiffs && !SPIFFS.exists(name))
    {
      Serial.println("Attempted to show gif but it doesnt exist on SPIFFS: path:" + String(name));
      return;
    }

    LoadGIF(name);
    gifPlaying = true;
    interruptGif = false;
  }

  if (interruptGif && gifPlaying)
  {
    gifPlaying = interruptGif = false;
    animGif.close();
    return;
  }


  lastResult = animGif.playFrame(true, NULL);

  if (lastResult == -1)
  {
    sd_state = UNMOUNTED;
  }

  if (lastResult == 0)
  {
    animGif.close();
    gifPlaying = false;
  }

} /* ShowGIF() */

void InitMatrixGif(VirtualMatrixPanel *panel)
{
  matrixGifPanel = panel;
  animGif.begin(LITTLE_ENDIAN_PIXELS);
}