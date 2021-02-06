#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>
#include <SPI.h>

#include "Global.h"
#include "Configuration.hpp"
#include "MatrixGif.hpp"
#include "Gifplayer.hpp"
#include "WebServer.hpp"
#include "Wifi.hpp"
#include "MatrixText.hpp"
#include "OTA.h"

#define SCK 33
#define MISO 32
#define MOSI 21
#define CS 22

// Config for 2 64x32 panels chained in a stacked config.
// Change MATRIX_WIDTH to 128 in ESP32-HUB75-MatrixPanel-I2S-DMA.h
#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW

MatrixPanel_I2S_DMA dma_display;
VirtualMatrixPanel virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true);

SPIClass sd_spi(HSPI);

frame_status_t frame_state = PLAYING_ART;
frame_status_t target_state = PLAYING_ART;
frame_status_t lastState = frame_state;
unsigned long lastStateChange = 0;

Config config;

void handleBrightness()
{
  ShowGIF("/bulb.gif", true);

  if (millis() - lastStateChange > 5000)
  {
    frame_state = PLAYING_ART;
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Pixel Art Frame");

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  Serial.println("Loading configuration");
  loadSettings();

  Serial.println("Init SD");
  sd_spi.begin(SCK, MISO, MOSI, CS);

  if (!SD.begin(CS, sd_spi))
  {
    Serial.println("Failed to open SD card");
    // TODO: Message on display
  }

  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  dma_display.setPanelBrightness(config.brightness);
  dma_display.setMinRefreshRate(200);

  //dma_display.begin(R1_PIN_DEFAULT, G1_PIN_DEFAULT, B1_PIN_DEFAULT, R2_PIN_DEFAULT, G2_PIN_DEFAULT, B2_PIN_DEFAULT, A_PIN_DEFAULT, B_PIN_DEFAULT, C_PIN_DEFAULT, D_PIN_DEFAULT, E_PIN_DEFAULT, LAT_PIN_DEFAULT, OE_PIN_DEFAULT, CLK_PIN_DEFAULT, FM6126A);
  dma_display.begin();

  virtualDisp.fillScreen(dma_display.color565(0, 0, 0));

  InitMatrixGif(&virtualDisp);

  File root = SPIFFS.open("/");
  File tmp = root.openNextFile();

  setupWifi();

  initServer();
}

void loop()
{
  if (target_state != frame_state)
  {
    frame_state = target_state;
  }

  if (lastState != frame_state)
  {
    lastState = frame_state;

    Serial.print("Changed state to: ");
    Serial.println(frame_state);
    lastStateChange = millis();
  }

  if (frame_state == PLAYING_ART)
  {
    playGif();
  }

  if (frame_state == SHOW_TEXT)
  {
    showText(text);
  }

  if (frame_state == CONNECT_WIFI)
  {
    connecting();
  }

  if (frame_state == ADJ_BRIGHTNESS)
  {
    handleBrightness();
  }

  if (config.wifiMode == WIFI_AP_STA)
  {
    handleDns();
  }

  if (frame_state == OTA_UPDATE)
  {
    handleOTA();
  }
}
