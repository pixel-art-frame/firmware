#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <Global.h>

class Loader
{
public:
    virtual String loadNextFile() { return ""; };

protected:
    Loader(){};

    File loadRoot()
    {
        return SD.open(GIF_DIR);
    }

};