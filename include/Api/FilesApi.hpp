#pragma once

#include <ESPAsyncWebServer.h>
#include <SD.h>
#include "Global.h"
#include "GifLoader/Indexed.hpp"

#define FILES_PAGINATION_SIZE 25

class FilesApi
{
public:
    void listFiles(AsyncWebServerRequest *request);
    void deleteFile(AsyncWebServerRequest *request);
    void handleFile(AsyncWebServerRequest *request);
    void resetIndex(AsyncWebServerRequest *request);
    void countIndex(AsyncWebServerRequest *request) { request->send(200, "text/plain", String(indexed.getIndexes().size()-1)); }
private: 
    void listFilesSequential(AsyncWebServerRequest *request);
    void listFilesIndexed(AsyncWebServerRequest *request);

    Indexed indexed;

    File paginationFile;
    int page = 0;

    std::vector<String> files;
    int currentFileIndex = 0;
};