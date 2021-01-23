#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include "Global.h"

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

// handles uploads, source: somewhere on github
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index)
    {
        logmessage = "Upload Start: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = SD.open("/" + filename, "w");
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

    dma_display.setPanelBrightness(value);
    interruptGif = true;

    // TODO: Show light bulb icon on matrix

    request->send(200);
}

void configureWebServer()
{
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<h1>TODO: Return webpage here</h1>");
    });

    server->on("/gif/name", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", currentGif.name());
    });

    server->on("/panel/brightness", handleBrightness);

    server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
            request->send(200);
        }, handleUpload);
}

void initServer()
{
    server = new AsyncWebServer(80);
    configureWebServer();

    // startup web server
    Serial.println("Starting Webserver ...");
    server->begin();
}
