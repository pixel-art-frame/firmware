#ifndef _GIFLOADER_
#define _GIFLOADER_


typedef enum {
    // Files are loaded one by one from the root.  
    SEQUENTIAL = 0,  

    /**
     * An indexing file is created to randomly load the GIFS
     * Best for large amounts of GIFS
     * 
     * For the best performance split the GIFS in directories.
     * Indexing 1000 files in a directory should take about 6 minutes
     */  
    INDEXED

} load_strategy_t;


bool queueEmpty();
void countTotalFiles();
void handleGifQueue();
String getNextGif();

#endif