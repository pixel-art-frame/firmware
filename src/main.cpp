#include <Arduino.h>
#include <FS.h>
#include "SdFat.h"
#include <SPIFFS.h>
#include <SPI.h>

#include "Global.h"
#include "Configuration.hpp"
#include "GifLoader.hpp"
#include "GifPlayer.hpp"
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

TaskHandle_t scheduled_task;

void handleBrightness()
{
  ShowGIF("/bulb.gif", true);

  if (millis() - lastStateChange > 1000)
  {
    target_state = PLAYING_ART;
  }
}

void handleScheduled(void *param)
{
  setupWifi();

  initServer();

  setupNTPClient();

  for (;;)
  {
    handleGifQueue();

    // Time
    if (config.enableTime)
    {
      if (millis() - lastTimeShow > config.timeInterval * 1000)
      {
        lastTimeShow = millis();
        target_state = SHOW_TIME;
      }

      // Update time2 secs before we should show it
      if ((millis() + 2000) - lastTimeShow > config.timeInterval * 1000)
      {
        Serial.println("updating time");
        updateTime();
      }
    }

    vTaskDelay(1 / portTICK_PERIOD_MS); // https://github.com/espressif/esp-idf/issues/1646#issue-299097720
  }
}

void setup()
{
  Serial.begin(115200);

  loadSettings();

  sd_spi.begin(SCK, MISO, MOSI, CS);

  sd_ready = SD.begin(CS, sd_spi);

  if (!sd_ready) {
    Serial.println("SD failed");
    ESP.restart();
    return;
  }

  Serial.println("SD card ready");

  if (!SPIFFS.begin())
  {
    // TODO: Handle this
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

  xTaskCreatePinnedToCore(
      handleScheduled, /* Function to implement the task */
      "schedule",      /* Name of the task */
      10000,           /* Stack size in words */
      NULL,            /* Task input parameter */
      0,               /* Priority of the task */
      &scheduled_task, /* Task handle. */
      0);              /* Core where the task should run */
}

void handleStartup()
{
  virtualDisp.clearScreen();
  virtualDisp.println("Pixel Art Frame");
  // TODO: Show boot logo

  delay(5000);

  if (frame_state == STARTUP && (frame_state != INDEXING && target_state != INDEXING))
  {
    Serial.println("Chanting to art after startup, frame state: " + String(frame_state) + " target state: " + String(target_state));
    target_state = PLAYING_ART;
  }
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

  if (frame_state == INDEXING && target_state != PLAYING_ART) {
    return false;
  }

  return true;
}

void loop()
{
  if (!sd_ready)
  {
    handleSdError();
    return;
  }

  if (!gifPlaying && target_state != frame_state && targetStateValid())
  {
    frame_state = target_state;
    lastStateChange = millis();
  }

  if (frame_state == OFF && !displayClear)
  {
    virtualDisp.fillScreen(dma_display.color565(0, 0, 0));
    displayClear = true;
  }

  if (frame_state == STARTUP)
  {
    handleStartup();
  }

  if (frame_state == PLAYING_ART)
  {
    handleGif();
  }

  if (frame_state == SHOW_TEXT)
  {
    handleText();
  }

  if (frame_state == SHOW_TIME)
  {
    handleTime();
  }

  if (frame_state == INDEXING)
  {
    virtualDisp.clearScreen();
    virtualDisp.setCursor(0, 0);
    virtualDisp.println("Indexing\nFiles: " + String(total_files));
    delay(200);
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
