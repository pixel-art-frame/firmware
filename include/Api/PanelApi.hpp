#pragma once

#include <ESPAsyncWebServer.h>
#include "Global.h"

class PanelApi
{
public:
    void handleBrightness(AsyncWebServerRequest *request);
};