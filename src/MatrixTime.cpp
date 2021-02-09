#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Global.h"
#include "MatrixText.hpp"

const long utcOffsetInSeconds = -14400;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// TODO: Move to config
// How long should we show the time?
int timeShowSeconds = 4;

void setupNTPClient()
{
    timeClient.begin();

    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    timeClient.setTimeOffset(3600);
}

void handleTime()
{
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

    println(time, virtualDisp.color565(0, 255, 0), 2, 1, 9, false, true, 0);
    println(date, virtualDisp.color565(0, 0, 255), 2, 3, 41, false, false, timeShowSeconds * 1000);
}