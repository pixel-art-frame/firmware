#include "GifLoader/Indexed.hpp"

String Indexed::generateIndexFilename(int index = 0)
{
    return String(INDEX_DIRECTORY) + "/" + String(INDEX_FILENAME_PREFIX) + String(index);
}

bool Indexed::indexingRequired()
{
    return !SD.exists(generateIndexFilename());
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

std::vector<String> Indexed::readIndexFile(File *indexFile)
{
    std::vector<String> files;
    String content = indexFile->readString();
    String line = "";

    for (int i = 0; i < INDEX_SIZE; i++)
    {
        line = getValue(content, '\n', i);
        if (line == "")
        {
            break;
        }

        files.push_back(line);
    }

    return files;
}

// Read random item from the index
String Indexed::loadNextFile()
{
    if (!indexesLoaded)
    {
        loadIndexes();
    }

    if (indexing || indexingRequired())
    {
        index();
        return "";
    }

    int index = random(0, indexes.size());

    File indexFile = SD.open(indexes.at(index));

    std::vector<String> files = readIndexFile(&indexFile);

    if (files.size() == 0)
        return "";

    int randomFile = random(0, (files.size() - 1));
    String f = files.at(randomFile);
    f.trim();
    f.replace("\n", "");

    indexFile.close();

    return f;
}

void Indexed::writeIndex()
{
    String fullPath = generateIndexFilename(currentIndex);

    if (SD.exists(fullPath))
    {
        SD.remove(fullPath);
    }

    File indexFile = SD.open(fullPath, FILE_WRITE);

    for (int i = 0; i < indexFiles.size(); i++)
    {
        indexFile.println(indexFiles.at(i));
    }

    indexFile.close();
    indexFiles.clear();

    indexes.push_back(fullPath);

    currentIndex++;
}

void Indexed::loadIndexes()
{
    if (!SD.exists(INDEX_DIRECTORY))
    {
        return;
    }

    File indexRoot = SD.open(INDEX_DIRECTORY);

    if (!indexRoot)
    {
        return;
    }

    for (;;)
    {
        File indexFile = indexRoot.openNextFile();

        if (!indexFile)
        {
            break;
        }

        indexes.push_back(String(indexFile.name()));

        indexFile.close();
    }

    indexesLoaded = true;
}

void Indexed::index()
{
    target_state = INDEXING;

    if (!curDirectory)
    {
        curDirectory = loadRoot();

        if (SD.exists(INDEX_DIRECTORY))
        {
            SD.rmdir(INDEX_DIRECTORY);
        }

        SD.mkdir(INDEX_DIRECTORY);

        indexing = true;
        total_files = 0;
        indexFiles.clear();
        while (!directories.empty())
            directories.pop();
    }

    File nextFile = curDirectory.openNextFile();

    if (!nextFile)
    {
        nextFile.close();
        curDirectory.close();

        if (!directories.empty())
        {
            String nextDir = directories.top();
            curDirectory = SD.open(nextDir);
            directories.pop();

            return;
        }

        if (indexFiles.size() > 0)
            writeIndex();

        target_state = PLAYING_ART;
        indexing = false;

        return;
    }

    if (nextFile.isDirectory())
    {
        String dirName = nextFile.name();
        directories.push(dirName);

        nextFile.close();
        return;
    }

    String nextFileName = String(nextFile.name());
    if (!nextFileName.endsWith(".gif"))
    {
        nextFile.close();
        return;
    }

    indexFiles.push_back(nextFileName);
    total_files++;
    nextFile.close();

    if (indexFiles.size() >= INDEX_SIZE)
    {
        writeIndex();
    }
}