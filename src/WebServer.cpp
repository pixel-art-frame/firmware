#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <SPIFFS.h>
#include "Global.h"
#include "GifPlayer.hpp"
#include "Configuration.hpp"
#include "MatrixText.hpp"
#include "Api/FilesApi.hpp"
#include "Api/PanelApi.hpp"
#include "Api/ConfigApi.hpp"

const char default_index[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
</head>
<body>
  <p><h1>Files missing!</h1></p>
  <form method="POST" action="/spiffs/upload" enctype="multipart/form-data"><input type="file" name="data"/><input type="submit" name="upload" value="Upload" title="Upload File"></form>
  <p>Please re-upload the required files</p>
</body>
</html>
)rawliteral";

AsyncWebServer *server;
FilesApi filesApi;
PanelApi panelApi;
ConfigApi configApi;

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes)
{
    if (bytes < 1024)
        return String(bytes) + " B";
    else if (bytes < (1024 * 1024))
        return String(bytes / 1024.0) + " KB";
    else if (bytes < (1024 * 1024 * 1024))
        return String(bytes / 1024.0 / 1024.0) + " MB";
    else
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

void handleAutoplay(AsyncWebServerRequest *request)
{
    if (!request->hasParam("value"))
    {
        request->send(400, "text/plain", "Missing parameter: state");
        return;
    }

    autoPlay = strcmp(request->getParam("value")->value().c_str(), "1") == 0;

    request->send(200, "text/plain", autoPlay ? "1" : "0");
}

void handlePlayGif(AsyncWebServerRequest *request)
{
    if (!request->hasParam("name"))
    {
        request->send(400, "text/plain", "Missing parameter: name");
        return;
    }

    String fileName = request->getParam("name")->value();

    if (!SD.exists(fileName))
    {
        request->send(400, "text/plain", "File does not exist");
        return;
    }

    setGif(fileName);

    request->send(400, "text/plain", "File not found");
}

// handles uploads, source: https://github.com/smford/esp32-asyncwebserver-fileupload-example
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index)
    {
        logmessage = "Upload Start: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = SD.open("/gifs/" + filename, "w");
        Serial.println(logmessage);
    }

    if (len)
    {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        Serial.println(logmessage);
    }

    if (final)
    {
        logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        Serial.println(logmessage);
        request->redirect("/");

        total_files++;
    }
}

void handleSpiffsUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index)
    {
        logmessage = "Upload Start: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = SPIFFS.open(filename, "w");
        Serial.println(logmessage);
    }

    if (len)
    {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        Serial.println(logmessage);
    }

    if (final)
    {
        logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        Serial.println(logmessage);
        request->redirect("/");
    }
}

void handleTextRequest(AsyncWebServerRequest *request)
{
    if (!request->hasParam("text"))
    {
        request->send(400, "text/plain", "Missing parameter: text");
        return;
    }

    request->send(200, "text/plain", "OK");

    interruptGif = true;

    text.text = request->getParam("text")->value();
    text.color = request->hasParam("color") ? atoi(request->getParam("color")->value().c_str()) : virtualDisp.color565(255, 0, 0);
    text.size = request->hasParam("size") ? atoi(request->getParam("size")->value().c_str()) : 1;

    text.x = request->hasParam("x") ? atoi(request->getParam("x")->value().c_str()) : 4;
    text.y = request->hasParam("y") ? atoi(request->getParam("y")->value().c_str()) : 4;

    text.wrap = request->hasParam("wrap") ? request->getParam("wrap")->value() == "1" : true;
    text.scroll = request->hasParam("scroll") ? request->getParam("scroll")->value() == "1" : false;
    text.clearScreen = request->hasParam("clearScreen") ? request->getParam("clearScreen")->value() == "1" : true;

    text.speed = request->hasParam("speed") ? atoi(request->getParam("speed")->value().c_str()) : 4;
    text.delay = request->hasParam("delay") ? atoi(request->getParam("delay")->value().c_str()) * 1000 : 5000;

    showText(text);

    target_state = SHOW_TEXT;
}

void handleTimeSettings(AsyncWebServerRequest *request)
{
    if (
        !request->hasParam("enable") ||
        !request->hasParam("interval") ||
        !request->hasParam("show") ||
        !request->hasParam("offset"))
    {
        request->send(400, "text/plain", "Missing parameter(s): enable, interval, show and offset");
        return;
    }

    config.enableTime = request->getParam("enable")->value() == "1";
    config.timeInterval = atoi(request->getParam("interval")->value().c_str());
    config.timeOffset = atoi(request->getParam("offset")->value().c_str());
    config.timeShowSeconds = atoi(request->getParam("show")->value().c_str());

    saveSettings();

    request->send(200, "text/plain", "Saved");
}

void handleGetTimeSettings(AsyncWebServerRequest *request)
{
    String response = "{";

    response += "\"enable\": " + String(config.enableTime) + ",";
    response += "\"interval\": " + String(config.timeInterval) + ",";
    response += "\"offset\": " + String(config.timeOffset) + ",";
    response += "\"show\": " + String(config.timeShowSeconds) + "}";

    request->send(200, "application/json", response);
}

void configureApiGifs()
{
    server->on("/gif/name", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", current_gif);
    });

    server->on("/gif/next", HTTP_GET, [](AsyncWebServerRequest *request) {
        nextGif();
        request->send(200, "text/plain", current_gif);
    });

    server->on("/gif/autoplay", HTTP_POST, handleAutoplay);
    server->on("/gif/autoplay", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", autoPlay ? "1" : "0");
    });

    server->on("/gif", HTTP_POST, handlePlayGif);
}

void configureApiTime()
{
    server->on("/time/show", HTTP_GET, [](AsyncWebServerRequest *request) {
        target_state = SHOW_TIME;
        interruptGif = true;
        request->send(200, "text/plain");
    });
    server->on("/time/settings", HTTP_POST, handleTimeSettings);
    server->on("/time/settings", HTTP_GET, handleGetTimeSettings);
}

void configureApiPanel()
{
    server->on("/panel/brightness", HTTP_POST, [](AsyncWebServerRequest *request) { panelApi.handleBrightness(request); });

    server->on("/panel/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(config.brightness));
    });
}

void configureApiFiles()
{
    server->on("/files", HTTP_GET, [](AsyncWebServerRequest *request) { filesApi.listFiles(request); });

    server->on("/file/delete", HTTP_GET, [](AsyncWebServerRequest *request) { filesApi.deleteFile(request); });
    server->on("/file", HTTP_GET, [](AsyncWebServerRequest *request) { filesApi.handleFile(request); });

    server->on("/index/reset", HTTP_GET, [](AsyncWebServerRequest *request) { filesApi.resetIndex(request); });
    server->on("/index/count", HTTP_GET, [](AsyncWebServerRequest *request) { filesApi.countIndex(request); });
    server->serveStatic("/index", SD, INDEX_DIRECTORY);

    server->on(
        "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
            request->send(200);
        },
        handleUpload);

    server->on(
        "/spiffs/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
            request->send(200);
        },
        handleSpiffsUpload);
}

void configureApiConfig()
{
    server->on("/config/wifi", HTTP_POST, [](AsyncWebServerRequest *request) { configApi.handleWifiConfig(request); });

    server->on("/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request) { configApi.handleScanWifi(request); });

    server->on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200);
        ESP.restart();
    });

    server->on("/ota", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Started OTA mode");

        target_state = OTA_UPDATE;
        interruptGif = true;
    });
}

void configureWebServer()
{
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SPIFFS.exists("/index.html"))
        {
            request->send_P(200, "text/html", default_index);
            return;
        }

        Serial.println("Returning index from spiffs");

        request->send(SPIFFS, "/index.html");
    });

    server->serveStatic("/_assets", SPIFFS, "/_assets/");

    configureApiConfig();
    configureApiPanel();
    configureApiGifs();
    configureApiFiles();
    configureApiTime();

    server->on("/text", HTTP_POST, handleTextRequest);

    server->on("/state", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("state"))
        {
            request->send(400, "text/plain", "Missing param: state");
            return;
        }

        int state = atoi(request->getParam("state")->value().c_str());

        if (state < 0 || state > OTA_UPDATE + 1)
        {
            request->send(400, "text/plain", "Must be between 0 and " + String(OTA_UPDATE + 1));
            return;
        }

        request->send(200, "text/plain", "Changing state to: " + String(state));

        target_state = (frame_status_t)state;
        interruptGif = true;
    });

    server->on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Core: " + String(xPortGetCoreID()));
    });
}

void initServer()
{
    server = new AsyncWebServer(80);
    configureWebServer();

    server->begin();
}
