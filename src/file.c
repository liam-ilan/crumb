#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "file.h"

// Normalize a path to be as simple as possible, relative to the current working dir.
// Makes similar files have the same path.
// ie. ./abc/def/../useable.crumb and abc/useable.crumb both become ./abc/useable.crumb.
// Allocates new memory.
char *normalizePath(char *path) {
  // Only normalize relative paths
  if (path[0] == '~' || path[0] == '/') {
      printf(
        "Error: Attempted to write %s to the use cache, "
        "paths in invocations to the use function must be relative, "
        "and the path cannot step out of the working directory.\n", path
      );
      exit(0);
  }

  // Copy that path for strtok_r.
  char *pathCopy = malloc(strlen(path) + 1);
  strcpy(pathCopy, path);

  // Keep track of the start for freeing at the end.
  char *pathCopyStart = pathCopy;

  // Allocate memory for the result.
  // The size of the result will at most be the size of the original path, 
  // + 2 for the "./" prefix.
  char *res = malloc(strlen(path) + 1 + 2);

  // res without the prefix ".".
  char *unprefixedRes = &res[1];
  res[0] = '.';
  res[1] = '/';

  // Index to write to next in unprefixedRes.
  int writeIndex = 0;
  char* token;

  // Split on "/".
  while ((token = strtok_r(pathCopy, "/", &pathCopy))) {
    if (strcmp(token, "..") == 0) {
      // If "..", backtrack to last "/".
      while (unprefixedRes[writeIndex] != '/') {
        writeIndex -= 1;

        // If we go too far back, fail.
        if (writeIndex < 0) {
          printf(
            "Error: Attempted to write %s to the use cache, "
            "paths in invocations to the use function must be relative, "
            "and the path cannot step out of the working directory.\n", path
          );
          exit(0);
        }
      }

      unprefixedRes[writeIndex] = '\0';
    } else if (!(strcmp(token, "") == 0 || strcmp(token, ".") == 0)){
      // Don't act on empty tokens or ".".
      // Add "/token".
      strcpy(&unprefixedRes[writeIndex], "/");
      writeIndex += 1;
      strcpy(&unprefixedRes[writeIndex], token);
      writeIndex += strlen(token);
    }
  }

  free(pathCopyStart);
  return res;
}


static FileCache fileCache = {
  .index = 0,
  .frozen = false
};

void FileCache_freeze() {
  fileCache.frozen = true;
}

CachedFile *FileCache_read(char *path) {
  char *normalizedPath = normalizePath(path);

  for (int i = 0; i < FILE_CACHE_SIZE; i++) {
    bool pathExists = fileCache.cache[i].path != NULL;
    if (pathExists && strcmp(fileCache.cache[i].path, normalizedPath) == 0) {
      free(normalizedPath);
      return &(fileCache.cache[i]);
    }
  }

  free(normalizedPath);
  return NULL;
}

void FileCache_write(char *path, char *contents, long fileLength) {
  // free item to write to
  if (fileCache.cache[fileCache.index].path != NULL) {
    free(fileCache.cache[fileCache.index].path);
  }
  if (fileCache.cache[fileCache.index].contents != NULL) {
    free(fileCache.cache[fileCache.index].contents);
  }

  // allocate new content/path to heap
  char *newContents = malloc(fileLength + 1);
  memcpy(newContents, contents, fileLength + 1);

  char *newPath = normalizePath(path);

  fileCache.cache[fileCache.index].path = newPath;
  fileCache.cache[fileCache.index].contents = newContents;
  fileCache.cache[fileCache.index].fileLength = fileLength;

  // progress the write index
  fileCache.index += 1;
  fileCache.index %= FILE_CACHE_SIZE;
}

void FileCache_free() {
  for (int i = 0; i < FILE_CACHE_SIZE; i++) {
    if (fileCache.cache[i].path != NULL) {
      free(fileCache.cache[i].path);
    }
    if (fileCache.cache[i].contents != NULL) {
      free(fileCache.cache[i].contents);
    }

    fileCache.cache[i].path = NULL;
    fileCache.cache[i].contents = NULL;
    fileCache.cache[i].fileLength = 0;
  }

  fileCache.index = 0;
}

char *readFile(char *path, bool cache) {
  if (cache) {
    CachedFile *p_cachedFile = FileCache_read(path);
    if (p_cachedFile != NULL) {
      char *res = malloc(p_cachedFile->fileLength + 1);
      memcpy(res, p_cachedFile->contents, p_cachedFile->fileLength + 1);
      return res;
    }
  }

  FILE *p_file = fopen(path, "r");

  // error handling
  if (p_file == NULL) {
    return NULL; 
  }

  // go to end, and record position (this will be the length of the file)
  int fseekRes = fseek(p_file, 0, SEEK_END);
  if (fseekRes != 0) {
    // handle fseek error
    return NULL;
  }

  long fileLength = ftell(p_file);

  // rewind to start
  rewind(p_file);

  // allocate memory (+1 for 0 terminated string)
  char *res = malloc(fileLength + 1);

  // read file and close
  fread(res, fileLength, 1, p_file);
  fclose(p_file);

  // set terminator to 0 and return
  res[fileLength] = 0;

  // write to the cache so we do not need to open a new reader next time
  if (cache && !fileCache.frozen) {
    FileCache_write(path, res, fileLength);
  }

  return res;
}

void writeFile(char *path, char* contents, int lineNumber) {
  FILE *p_file = fopen(path, "w+");

  // check the file succesfully opened
  if (p_file == NULL) {
    printf(
      "Runtime Error @ Line %i: Cannot write file \"%s\".\n", 
      lineNumber, path
    );
    exit(0); 
  }

  fprintf(p_file, "%s", contents);
  fclose(p_file);
}