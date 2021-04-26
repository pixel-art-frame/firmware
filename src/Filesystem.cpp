#include "Filesystem.hpp"
#include <FS.h>
#include "SD.h"
#include <SPIFFS.h>
#include <SPI.h>
#include "Global.h"
#include "MatrixGif.hpp"

#if PANEL_128_32
#define INSERT_SD_GIF "/insert_sd_128x32.gif"
#else
#define INSERT_SD_GIF "/insert_sd_64x64.gif"
#endif

#define FS_MOUNT_ATTEMPTS 3
#define FS_MOUNT_RETRY 200

#define SCK 33
#define MISO 32
#define MOSI 21
#define CS 22

sd_state_t sd_state = UNMOUNTED;

SPIClass sd_spi(HSPI);

void unmount_sd()
{
    if (sd_state != MOUNTED)
    {
        return;
    }

    SD.end();
    sd_spi.end();
    sd_state = UNMOUNTED;
}

bool mount_sd()
{
    if (sd_state == MOUNTED)
    {
        return true; // Already mounted
    }

     sd_spi.begin(SCK, MISO, MOSI, CS);

    int mount_attempts = 0;

    while (!SD.begin(CS, sd_spi))
    {
        mount_attempts++;

        SD.end();

        if (mount_attempts > FS_MOUNT_ATTEMPTS)
        {
            sd_state = MOUNT_FAILED;
            return false;
        }

        delay(FS_MOUNT_RETRY);
    }

    sd_state = MOUNTED;

    return true;
}

bool mount_spiffs()
{
    int mount_attempts = 0;
    bool format = false;

    while (!SPIFFS.begin(format))
    {
        mount_attempts++;

        SD.end();

        if (mount_attempts > FS_MOUNT_ATTEMPTS)
        {
            if (format)
                return false;

            // Retry again but format
            mount_attempts = 0;
            format = true;
        }

        delay(FS_MOUNT_RETRY);
    }

    return true;
}

bool handle_sd_error()
{
    unmount_sd();

    if (mount_sd())
    {
        return true;
    }

    ShowGIF(INSERT_SD_GIF, true);

    return false;
}


bool mount_fs()
{
    return mount_spiffs() && mount_sd();
}