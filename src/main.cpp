#include <Arduino.h>

#include "MatrixGif.hpp"

// Config for 2 64x32 panels chained in a stacked config.
// Change MATRIX_WIDTH to 128 in ESP32-HUB75-MatrixPanel-I2S-DMA.h
#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW

MatrixPanel_I2S_DMA dma_display;
VirtualMatrixPanel virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Pixel Art Frame");

  dma_display.setPanelBrightness(64);
  dma_display.setMinRefreshRate(200);

  dma_display.begin(R1_PIN_DEFAULT, G1_PIN_DEFAULT, B1_PIN_DEFAULT, R2_PIN_DEFAULT, G2_PIN_DEFAULT, B2_PIN_DEFAULT, A_PIN_DEFAULT, B_PIN_DEFAULT, C_PIN_DEFAULT, D_PIN_DEFAULT, E_PIN_DEFAULT, LAT_PIN_DEFAULT, OE_PIN_DEFAULT, CLK_PIN_DEFAULT, FM6126A);

  virtualDisp.fillScreen(dma_display.color565(0, 0, 0));

  Serial.println(" * Loading SPIFFS");
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");
    // TODO: Error on display
  }

  InitMatrixGif(&virtualDisp);
}

char gifName[256];

void playGif()
{
  char *gifDir = "/"; // play all GIFs in this directory on the SD card/SPIFFS
  File root, temp; // TODO: Extract root and temp to global vars

  root = FILESYSTEM.open(gifDir);
  if (!root)
  {
    Serial.printf("Failed to open GIF directory");
    // TODO: Message on display
    return;
  }

  temp = root.openNextFile();
  while (temp)
  {
    if (temp.isDirectory())
    {
      temp.close();
      temp = root.openNextFile();
    }

    strcpy(gifName, temp.name());

    Serial.printf("Playing %s\n", temp.name());
    Serial.flush();
    ShowGIF((char *)temp.name());
  }
  root.close();
}

void loop()
{
  // put your main code here, to run repeatedly:
}