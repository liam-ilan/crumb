#ifndef FILE_H
#define FILE_H
#define FILE_CACHE_SIZE 256

// an individual cached file, both path and contents are heap allocated
typedef struct CachedFile {
    char *path;
    char *contents;
    long fileLength;
} CachedFile;

// cache of files that were read
typedef struct FileCache {
    int index;
    CachedFile cache[FILE_CACHE_SIZE];
} FileCache;

// prototypes
char *readFile(char *);
void writeFile(char *, char *, int);
void FileCache_free();

#endif