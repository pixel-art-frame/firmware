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

private:
    void listFilesSequential(AsyncWebServerRequest *request);
    void listFilesIndexed(AsyncWebServerRequest *request);

    Indexed indexed;

    File paginationFile;
    int page = 1;
};