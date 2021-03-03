#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPIFFS.h>
#include <SPI.h>

#include "Global.h"
#include "Configuration.hpp"
#include "Gifplayer.hpp"
#include "WebServer.hpp"
#include "Wifi.hpp"
#include "MatrixGif.hpp"
#include "MatrixText.hpp"
#include "MatrixTime.hpp"
#include "OTA.h"

#define SCK 33
#define MISO 32
#define MOSI 21
#define CS 22

#define PANEL_128_32 true
#define FM6126A_PANEL false

#if PANEL_128_32

// Config for 2 64x32 panels chained in a stacked config.
// Change MATRIX_WIDTH to 128 in ESP32-HUB75-MatrixPanel-I2S-DMA.h
#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 2 // Number of INDIVIDUAL PANELS per ROW

#else

// Config for 2 64x32 panels chained in a stacked config.
// Change MATRIX_WIDTH to 128 in ESP32-HUB75-MatrixPanel-I2S-DMA.h
#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW

#endif

MatrixPanel_I2S_DMA dma_display;
VirtualMatrixPanel virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true);

// Flag for the OFF state
bool displayClear = false;

SPIClass sd_spi(HSPI);
bool sd_ready = false;

frame_status_t frame_state = STARTUP;
frame_status_t target_state = STARTUP;
frame_status_t lastState = frame_state;
unsigned long lastStateChange = 0;

Config config;

void handleBrightness()
{
  ShowGIF("/bulb.gif", true);

  if (millis() - lastStateChange > 1000)
  {
    target_state = PLAYING_ART;
  }
}

void handleScheduled()
{
  // Time
  if (config.enableTime && millis() - lastTimeShow > config.timeInterval * 1000)
  {
    lastTimeShow = millis();
    target_state = SHOW_TIME;
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

  sd_ready = SD.begin(CS, sd_spi);

  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  dma_display.setPanelBrightness(config.brightness);
  dma_display.setMinRefreshRate(200);

#if FM6126A_PANEL
  dma_display.begin(R1_PIN_DEFAULT, G1_PIN_DEFAULT, B1_PIN_DEFAULT, R2_PIN_DEFAULT, G2_PIN_DEFAULT, B2_PIN_DEFAULT, A_PIN_DEFAULT, B_PIN_DEFAULT, C_PIN_DEFAULT, D_PIN_DEFAULT, E_PIN_DEFAULT, LAT_PIN_DEFAULT, OE_PIN_DEFAULT, CLK_PIN_DEFAULT, FM6126A);
#else
  dma_display.begin();
#endif

  virtualDisp.fillScreen(dma_display.color565(0, 0, 0));

  InitMatrixGif(&virtualDisp);

  setupWifi();

  initServer();

  setupNTPClient();

  target_state = PLAYING_ART;
}

void handleSdError()
{
  SD.end();
  sd_ready = SD.begin(CS, sd_spi);

  if (sd_ready)
    return;

  println("SD Failed\nInsert SD", dma_display.color565(255, 0, 0), 1, 1, 1, true, true, 1000);
}

bool targetStateValid()
{
  if (target_state == PLAYING_ART && !sd_ready)
  {
    return false;
  }

  return true;
}

void loop()
{
  handleScheduled();

  if (!sd_ready) {
    handleSdError();
  }

  if (target_state != frame_state && targetStateValid())
  {
    frame_state = target_state;
    lastStateChange = millis();
  }

  if (frame_state == OFF && !displayClear)
  {
    virtualDisp.fillScreen(dma_display.color565(0, 0, 0));
    displayClear = true;
  }

  if (frame_state == PLAYING_ART)
  {
    playGif();
  }

  if (frame_state == SHOW_TEXT)
  {
    handleText();
  }

  if (frame_state == SHOW_TIME)
  {
    handleTime();
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
