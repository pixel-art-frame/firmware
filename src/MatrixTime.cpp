#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Global.h"
#include "MatrixText.hpp"

#define TIME_X 2
#define TIME_Y 9

#if PANEL_128_32

#define DATE_X 3
#define DATE_Y 41

#else

#define DATE_X 64
#define DATE_Y 9

#endif

const int utcOffsetInSeconds = -14400;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setupNTPClient()
{
    if (!config.enableTime)
        return;

    timeClient.begin();

    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    timeClient.setTimeOffset(config.timeOffset);
}

void handleTime()
{
    if (!config.enableTime)
        return;

    while (!timeClient.update())
    {
        timeClient.forceUpdate();
    }

    unsigned long epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);

    int hour = timeClient.getHours();
    int min = timeClient.getMinutes();
    int day = ptm->tm_mday;
    int month = ptm->tm_mon + 1;

    String hourStr = hour < 10 ? "0" + String(hour) : String(hour);
    String minStr = min < 10 ? "0" + String(min) : String(min);

    String time = hourStr + ":" + minStr;

    String dayStr = day < 10 ? "0" + String(day) : String(day);
    String monthStr = month < 10 ? "0" + String(month) : String(month);
    String date = dayStr + "/" + monthStr;

    println(time, virtualDisp.color565(0, 255, 0), 2, TIME_X, TIME_Y, false, true, 0);
    println(date, virtualDisp.color565(0, 0, 255), 2, DATE_X, DATE_Y, false, false, config.timeShowSeconds * 1000);
}