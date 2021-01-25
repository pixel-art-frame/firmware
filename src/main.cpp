#include <Arduino.h>
#include <WiFiManager.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "Global.h"
#include "MatrixGif.hpp"
#include "Gifplayer.hpp"
#include "WebServer.hpp"

#define SCK 33
#define MISO 32
#define MOSI 21
#define CS 22

WiFiManager wifiManager;

// Config for 2 64x32 panels chained in a stacked config.
// Change MATRIX_WIDTH to 128 in ESP32-HUB75-MatrixPanel-I2S-DMA.h
#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW

MatrixPanel_I2S_DMA dma_display;
VirtualMatrixPanel virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true);
SPIClass sd_spi(HSPI);

int brightness = 20;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Pixel Art Frame");

  Serial.println("Init SD");
  sd_spi.begin(SCK, MISO, MOSI, CS);

  if (!SD.begin(CS, sd_spi))
  {
    Serial.println("Failed to open SD card");
    // TODO: Message on display
  }

  wifiManager.autoConnect("Pixel Art Frame", "PixelArt");
  initServer();

  dma_display.setPanelBrightness(brightness);
  dma_display.setMinRefreshRate(200);

  dma_display.begin(R1_PIN_DEFAULT, G1_PIN_DEFAULT, B1_PIN_DEFAULT, R2_PIN_DEFAULT, G2_PIN_DEFAULT, B2_PIN_DEFAULT, A_PIN_DEFAULT, B_PIN_DEFAULT, C_PIN_DEFAULT, D_PIN_DEFAULT, E_PIN_DEFAULT, LAT_PIN_DEFAULT, OE_PIN_DEFAULT, CLK_PIN_DEFAULT, FM6126A);

  virtualDisp.fillScreen(dma_display.color565(0, 0, 0));

  InitMatrixGif(&virtualDisp);
}


void loop()
{
  playGif();
    
}
