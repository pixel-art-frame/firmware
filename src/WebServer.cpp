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

void handleWifiConfig(AsyncWebServerRequest *request)
{
    Serial.println("Wifi config");

    if (!request->hasParam("ssid") || !request->hasParam("pass"))
    {
        request->send(400, "text/plain", "Missing parameter(s): ssid, value");
        return;
    }

    config.ssid = request->getParam("ssid")->value();
    config.pass = request->getParam("pass")->value();
    saveSettings();

    request->send(200, "text/plain", "Restart to apply changes");
}

void handleBrightness(AsyncWebServerRequest *request)
{
    Serial.println("set panel brightness");

    if (!request->hasParam("value"))
    {
        request->send(400, "text/plain", "Missing parameter: value");
        return;
    }

    int value = atoi(request->getParam("value")->value().c_str());

    if (value <= 1 || value >= 255)
    {
        request->send(400, "invalid value");
        return;
    }

    config.brightness = value;
    saveSettings();
    dma_display.setPanelBrightness(config.brightness);
    interruptGif = true;

    frame_state = ADJ_BRIGHTNESS;

    request->send(200, "text/plain", String(config.brightness));
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

    const char *fileName = request->getParam("name")->value().c_str();

    if (!SD.exists(fileName))
    {
        request->send(400, "text/plain", "File does not exist");
        return;
    }

    for (int i = 0; i < gifs.size(); i++)
    {
        if (strcmp(gifs[i].c_str(), fileName) == 0)
        {
            setGif(i);
            request->send(200, "text/plain", String(fileName));
            return;
        }
    }

    request->send(400, "text/plain", "File not found");
}

void listFiles(AsyncWebServerRequest *request)
{
    String jsonResponse = "[";

    for (auto &gif : gifs)
    {
        jsonResponse += "\"" + gif + "\",";
    }

    request->send(200, "application/json", jsonResponse.substring(0, jsonResponse.length() - 1) + "]");
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

        gifsLoaded = false;
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

        gifsLoaded = false;
    }
}

void handleFile(AsyncWebServerRequest *request)
{
    if (!request->hasParam("name"))
    {
        request->send(400, "text/plain", "Missing parameter: name");
        return;
    }

    const char *fileName = request->getParam("name")->value().c_str();

    if (!SD.exists(fileName))
    {
        request->send(400, "text/plain", "File does not exist");
        return;
    }

    request->send(SD, fileName, "application/octet-stream");
}

void deleteFile(AsyncWebServerRequest *request)
{
    if (!request->hasParam("name"))
    {
        request->send(400, "text/plain", "Missing parameter: name");
        return;
    }

    const char *fileName = request->getParam("name")->value().c_str();

    if (!SD.exists(fileName))
    {
        request->send(400, "text/plain", "File does not exist");
        return;
    }

    SD.remove(fileName);
    gifsLoaded = false;
    request->send(200, "text/plain", "Deleted File: " + String(fileName));
}

void configureWebServer()
{
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SPIFFS.exists("index.html"))
        {
            request->send_P(200, "text/html", default_index);
            return;
        }

        request->send(SPIFFS, "index.html");
    });

    server->on("/gif/name", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(getCurrentGif()));
    });

    server->on("/gif/next", HTTP_GET, [](AsyncWebServerRequest *request) {
        nextGif();
        request->send(200, "text/plain", String(getCurrentGif()));
    });

    server->on("/gif/prev", HTTP_GET, [](AsyncWebServerRequest *request) {
        prevGif();
        request->send(200, "text/plain", String(getCurrentGif()));
    });

    server->on("/gif/autoplay", HTTP_POST, handleAutoplay);
    server->on("/gif/autoplay", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", autoPlay ? "1" : "0");
    });

    server->on("/gif", HTTP_POST, handlePlayGif);

    server->on("/panel/brightness", HTTP_POST, handleBrightness);
    server->on("/panel/brightness", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(config.brightness)); // TODO: Return whole config here
    });

    server->on("/files", listFiles);
    server->on("/file/delete", deleteFile);
    server->on("/file", handleFile);

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

    server->on("/config/wifi", HTTP_POST, handleWifiConfig);
    server->on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200);
        ESP.restart();
    });
}

void initServer()
{
    server = new AsyncWebServer(80);
    configureWebServer();

    server->begin();
}
