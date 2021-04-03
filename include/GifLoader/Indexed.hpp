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
    std::vector<String> getIndexes() { if (indexes.size() == 0) loadIndexes(); return indexes; }
    std::vector<String> readIndexFile(File *indexFile);

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