#include "Api/PanelApi.hpp"

void PanelApi::handleBrightness(AsyncWebServerRequest *request)
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

    target_state = ADJ_BRIGHTNESS;

    request->send(200, "text/plain", String(config.brightness));
}