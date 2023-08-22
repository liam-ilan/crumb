#include <stdlib.h>
#include <stdio.h>
#include "file.h"

char *readFile(char *path, int lineNumber) {
  FILE *p_file = fopen(path, "r");

  // error handling
  if (p_file == NULL) {
    printf(
      "Runtime Error @ Line %i: Cannot read file \"%s\".\n", 
      lineNumber, path
    );
    exit(0); 
  }

  // go to end, and record position (this will be the length of the file)
  fseek(p_file, 0, SEEK_END);
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