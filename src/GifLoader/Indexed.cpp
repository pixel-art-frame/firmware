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
    Serial.println("Loading next file");
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

    int randomFile = random(0, files.size());
    String f = files.at(randomFile);
    f.trim();
    f.replace("\n", "");

    indexFile.close();

    Serial.println("Returning file " + f);
    return f;
}

void Indexed::writeIndex()
{
    String fullPath = generateIndexFilename(currentIndex);

    Serial.println("Writing index to file " + fullPath + " file count " + String(indexFiles.size()));

    if (SD.exists(fullPath))
    {
        Serial.println("Index already exists, deleting");
        SD.remove(fullPath);
    }

    File indexFile = SD.open(fullPath, FILE_WRITE);

    for (int i = 0; i < indexFiles.size(); i++)
    {
        Serial.println("Added file " + String(i) + " to index: " + indexFiles.at(i));
        indexFile.println(indexFiles.at(i));
    }

    indexFile.close();
    indexFiles.clear();

    indexes.push_back(fullPath);

    currentIndex++;
}

void Indexed::loadIndexes()
{
    Serial.println("Loading indexes");
    if (!SD.exists(INDEX_DIRECTORY))
    {
        return;
    }

    File indexRoot = SD.open(INDEX_DIRECTORY);

    for (;;)
    {
        File indexFile = indexRoot.openNextFile();

        if (!indexFile)
        {
            break;
        }

        Serial.println("Found file: " + String(indexFile.name()));

        indexes.push_back(String(indexFile.name()));

        indexFile.close();
    }

    Serial.println("Indexes loaded!");

    indexesLoaded = true;
}

void Indexed::index()
{
    Serial.println("Indexing");
    target_state = INDEXING;

    if (!curDirectory)
    {
        if (total_files > 0)
            while (true)
                ;
        Serial.println("Start, loading root");

        curDirectory = loadRoot();

        if (SD.exists(INDEX_DIRECTORY))
        {
            Serial.println("Index dir already exists, deleting");

            SD.rmdir(INDEX_DIRECTORY);
        }

        Serial.println("Creating index directory");
        SD.mkdir(INDEX_DIRECTORY);

        indexing = true;
        total_files = 0;
        indexFiles.clear();
    }

    Serial.println("Opening next file from dir " + String(curDirectory.name()));

    File nextFile = curDirectory.openNextFile();

    if (!nextFile)
    {
        Serial.println("No more files");

        curDirectory.close();

        if (!directories.empty())
        {
            curDirectory = directories.top();
            directories.pop();

            Serial.println("We were in a subdirectory, going back to " + String(curDirectory.name()));
            return;
        }

        writeIndex();

        Serial.println("Indexing done!");
        target_state = PLAYING_ART;
        indexing = false;

        return;
    }

    if (nextFile.isDirectory())
    {
        File curDir = curDirectory;
        directories.push(curDir);

        curDirectory = nextFile;

        Serial.println("Found directory " + String(curDirectory.name()));

        return;
    }

    String nextFileName = String(nextFile.name());

    if (!nextFileName.endsWith(".gif"))
    {
        nextFile.close();
        return;
    }

    indexFiles.push_back(nextFileName);
    Serial.println("Added to current index: " + nextFileName + " Indexed files: " + String(indexFiles.size()));
    total_files++;

    nextFile.close();

    if (indexFiles.size() >= INDEX_SIZE)
    {
        writeIndex();
    }
}