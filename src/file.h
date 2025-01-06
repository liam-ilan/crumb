#ifndef FILE_H
#define FILE_H
#define FILE_CACHE_SIZE 1024
#include <stdbool.h>

// an individual cached file, both path and contents are heap allocated
typedef struct CachedFile {
  char *path;
  char *contents;
  long fileLength;
} CachedFile;

// cache of files that were read
typedef struct FileCache {
  int index;
  bool frozen;
  CachedFile cache[FILE_CACHE_SIZE];
} FileCache;

// prototypes
char *readFile(char *, bool);
void writeFile(char *, char *, int);
void FileCache_free();
void FileCache_write(char *path, char *contents, long fileLength);
void FileCache_freeze();

#endif