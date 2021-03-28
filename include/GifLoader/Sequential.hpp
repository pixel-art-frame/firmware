#pragma once

#include <stack>
#include <FS.h>
#include <SdFat.h>
#include "Loader.hpp"

class Sequential : public Loader
{
public:
    String loadNextFile();

protected:
    File curDirectory;
    std::stack<File> directories;
};