#include <queue>
#include <SD.h>
#include "Global.h"

#define MAX_QUEUED_GIFS 32
#define GIF_DIR "/gifs"

std::queue<String> gif_queue;
uint8_t min_skip = 0, max_skip = 0;
bool queue_populate_requred = true,
     total_files_counted = false;

unsigned long total_files = 0;

File root, fileCounterRoot;

void resetGifLoader()
{
    min_skip = max_skip = total_files_counted = total_files = 0;
    queue_populate_requred = false;

    std::queue<String> empty;
    std::swap(gif_queue, empty);
}

/**
 * Calculate the max and min skip which is used for some 'randomness'
 * while counting to total amount of files.
 * After that the random() will take over and the min/max skips become irrelevant
 */
void calculateSkip()
{
    min_skip = total_files * 0.05;
    max_skip = total_files * 0.3;
}

void countTotalFiles()
{
    if (total_files_counted)
        return;

    if (!fileCounterRoot)
    {
        fileCounterRoot = SD.open(GIF_DIR);
    }

    File curFile = fileCounterRoot.openNextFile();

    if (!curFile)
    {
        total_files_counted = true;
        fileCounterRoot.close();
        return;
    }

    if (curFile.isDirectory())
    {
        return;
    }

    total_files++;
    calculateSkip();
}

/**
 * Add a single item to the queue
 */
void populateGifQueue()
{
    if (!root)
    {
        root = SD.open(GIF_DIR);
    }

    File current_file;

    uint8_t skip = total_files_counted ? random(0, total_files) : random(min_skip, max_skip),
            skipped = 0;

    do
    {
        if (current_file)
            current_file.close();

        current_file = root.openNextFile();

        if (!current_file)
        {
            root.close();

            if (gif_queue.size() == 0)
            {
                populateGifQueue();
            }

            return;
        }

        skipped++;
    } while (skipped < skip);

    gif_queue.push(String(current_file.name()));
    current_file.close();
}

void handleGifQueue()
{
    if (!sd_ready)
        resetGifLoader();

    queue_populate_requred = gif_queue.size() <= MAX_QUEUED_GIFS;
    if (!queue_populate_requred)
        return;

    populateGifQueue();
}

String getNextGif()
{
    String gif = gif_queue.front();
    gif_queue.pop();

    return gif;
}