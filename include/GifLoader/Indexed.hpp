#pragma once

#include <stack>
#include <FS.h>
#include <SdFat.h>
#include <ArduinoJson.h>
#include "Loader.hpp"

class Indexed : public Loader
{
public:
    String loadNextFile();

protected:
    String generateIndexFilename(int);
    bool indexingRequired();

    void writeIndex();
    void loadIndexes();
    
    void index();
    
    std::vector<String> indexFiles;
    File curDirectory;
    std::stack<File> directories;
    int currentIndex = 0;


    std::vector<String> indexes;
    bool indexesLoaded = false;
    bool indexing = false;
};