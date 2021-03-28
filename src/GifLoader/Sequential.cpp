#include "GifLoader/Sequential.hpp"

String Sequential::loadNextFile()
{
    if (!curDirectory)
    {
        curDirectory = loadRoot();
    }

    File nextFile = curDirectory.openNextFile();

    if (!nextFile)
    {
        curDirectory.close();

        if (!directories.empty())
        {
            curDirectory = directories.top();
            directories.pop();
        }

        return loadNextFile();
    }

    if (nextFile.isDirectory())
    {
        File curDir = curDirectory;
        directories.push(curDir);

        curDirectory = nextFile;
        return loadNextFile();
    }

    String nextFileName = String(nextFile.name());

    nextFile.close();

    return nextFileName;
}