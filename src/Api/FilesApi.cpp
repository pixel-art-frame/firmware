#include "Api/FilesApi.hpp"

void FilesApi::listFiles(AsyncWebServerRequest *request)
{
    if (config.loadStrategy == SEQUENTIAL)
    {
        listFilesSequential(request);
    }

    if (config.loadStrategy == INDEXED)
    {
        // listFilesIndexed(request, globIndexedGifLoader);
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
    int maxPage = indexed.getIndexes().size();

    if (page > maxPage || request->hasParam("firstPage"))
    {
        page = 1;
    }

    String indexFilePath = indexed.getIndexes().at(page);
    File indexFile = SD.open(indexFilePath);

    std::vector<String> files = indexed.readIndexFile(&indexFile);

    String jsonResponse = "{\"page\":" + String(page) + ",\"pageSize\":" + String(INDEX_SIZE) + ",\"files\":[";

    for (int i = 0; i < files.size(); i++)
    {
        jsonResponse += "\"" + files.at(i) + "\",";
    }

    indexFile.close();

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
