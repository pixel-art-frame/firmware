#include "Api/FilesApi.hpp"

void FilesApi::listFiles(AsyncWebServerRequest *request)
{
    Serial.print("list files ");
    if (config.loadStrategy == SEQUENTIAL)
    {
        Serial.println("sequential");
        listFilesSequential(request);
    }

    if (config.loadStrategy == INDEXED)
    {
        Serial.println("indexed");

        listFilesIndexed(request);
    }
}

void FilesApi::listFilesSequential(AsyncWebServerRequest *request)
{
    if (request->hasParam("firstPage") && paginationFile)
    {
        paginationFile.close();
    }

    if (!paginationFile)
    {
        page = 1;
        paginationFile = SD.open(GIF_DIR);
    }
    else
    {
        page++;
    }

    int maxPage = (total_files / FILES_PAGINATION_SIZE) + 1;

    if (page > maxPage)
        page = maxPage;

    String jsonResponse = "{\"page\":" + String(page) + ",\"pageSize\":" + String(FILES_PAGINATION_SIZE) + ",\"files\":[";

    for (int i = 0; i < FILES_PAGINATION_SIZE; i++)
    {
        File f = paginationFile.openNextFile();

        if (!f)
        {
            paginationFile.close();
            break;
        }

        jsonResponse += "\"" + String(f.name()) + "\",";

        f.close();
    }

    request->send(200, "application/json", jsonResponse.substring(0, jsonResponse.length() - 1) + "]}");
}

void FilesApi::listFilesIndexed(AsyncWebServerRequest *request)
{
    Serial.println("list files indexed");
    int maxPage = indexed.getIndexes().size();
    
    Serial.println("max page: " + String(maxPage));

    if (page >= maxPage || request->hasParam("firstPage"))
    {
        Serial.println("Resetting to page 1");
        page = 1;
    }

    String indexFilePath = indexed.getIndexes().at(page);
    Serial.println("Index file: " + indexFilePath);
    File indexFile = SD.open(indexFilePath);
    Serial.println("Opened index file");
    std::vector<String> files = indexed.readIndexFile(&indexFile);

    Serial.println("Reading lines, count: " + String(files.size()));

    String jsonResponse = "{\"page\":" + String(page) + ",\"pageSize\":" + String(INDEX_SIZE) + ",\"files\":[";

    for (int i = 0; i < files.size(); i++)
    {
        jsonResponse += "\"" + files.at(i) + "\",";
    }

    indexFile.close();
    page++;

    request->send(200, "application/json", jsonResponse.substring(0, jsonResponse.length() - 1) + "]}");
}

void FilesApi::deleteFile(AsyncWebServerRequest *request)
{
    if (!request->hasParam("name"))
    {
        request->send(400, "text/plain", "Missing parameter: name");
        return;
    }

    const char *fileName = request->getParam("name")->value().c_str();

    if (!SD.exists(fileName))
    {
        request->send(400, "text/plain", "File does not exist");
        return;
    }

    SD.remove(fileName);
    total_files--;
    request->send(200, "text/plain", "Deleted File: " + String(fileName));
}

void FilesApi::handleFile(AsyncWebServerRequest *request)
{
    if (!request->hasParam("name"))
    {
        request->send(400, "text/plain", "Missing parameter: name");
        return;
    }

    const char *fileName = request->getParam("name")->value().c_str();

    if (!SD.exists(fileName))
    {
        request->send(400, "text/plain", "File does not exist");
        return;
    }

    request->send(SD, fileName, "application/octet-stream");
}

void FilesApi::resetIndex(AsyncWebServerRequest *request)
{
    if (config.loadStrategy != INDEXED) {
        request->send(400, "text/plain", "Load strategy not set to indexed");
        return;
    }

    Indexed indexed;
    auto indexes = indexed.getIndexes();
    for (int i = 0; i < indexes.size(); i++) {
        SD.remove(indexes.at(i));
        Serial.println("Deleted " + indexes.at(i));
    }

    if (SD.exists(INDEX_DIRECTORY)) {
        SD.rmdir(INDEX_DIRECTORY);
        Serial.println("Removed index dir");
    }

    request->send(200, "text/plain", "Indexes deleted");
}