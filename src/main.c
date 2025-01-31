#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include "tokens.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"
#include "generic.h"
#include "scope.h"
#include "eval.h"
#include "stdlib.h"
#include "events.h"
#include "file.h"
#include "run.h"

#define CRUMB_VERSION ("v0.0.4")

int main(int argc, char *argv[]) {

  // parse version flag
  if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
    printf("%s\n", CRUMB_VERSION);
    return 0;
  }

  // parse debug flag
  bool debug = argc >= 2 && strcmp(argv[1], "-d") == 0;

  /* read file */
  if (debug) printf("\nCODE\n");
  char *code = NULL;
  long fileLength = 0;
  bool pipedInput = false;

  // Check if stdin is empty.
  fseek(stdin, 0, SEEK_END);
  bool stdinEmpty = ftell(stdin) == 0;
  rewind(stdin);


  if ((debug && argc >= 3) || (!debug && argc >= 2)) {

    // if a path was supplied
    char *codePath = debug ? argv[2] : argv[1];

    // if code is passed through an file argument
    FILE *p_file = fopen(codePath, "r");
    if (p_file == NULL) {
      printf("Error: Could not read file %s.\n", codePath);
      return 0;
    }

    // go to end, and record position (this will be the length of the file)
    fseek(p_file, 0, SEEK_END);
    fileLength = ftell(p_file);

    // rewind to start
    rewind(p_file);

    // allocate memory (+1 for 0 terminated string)
    code = malloc(fileLength + 1);

    // read file and close
    fread(code, fileLength, 1, p_file); 
    fclose(p_file);

    // set terminator to 0
    code[fileLength] = '\0';

  } else if (!stdinEmpty) {
    
    // if stdin is empty, read it (code was passed in as a pipe)
    code = malloc(0);

    // finalIndex tracks the last index populated with a non '\0' char
    int finalIndex = 0;
    char c = EOF;

    // loop through stdin.
    for (finalIndex = 0; (c = getchar()) != EOF; finalIndex += 1) {
      code = realloc(code, finalIndex + 1);
      code[finalIndex] = c;
    }

    // create space for null terminator
    code = realloc(code, finalIndex + 2);
    fileLength = finalIndex + 1;
    code[fileLength] = '\0';

    pipedInput = true;
  } else {
    printf("Error: Program not supplied through pipe or argument.\n");
    return 0;
  }
  
  if (debug) {
    printf("%s\n", code);
  }

  // Calculate the number of arguments to skip (ie. name of executable, file passed).
  int argsToSkip = 1 + (pipedInput ? 0 : 1) + (debug ? 1 : 0);
  int exitCode = run(code, fileLength, argc - argsToSkip, &argv[argsToSkip], debug);

  // free code
  free(code);
  code = NULL;
  if (debug) printf("Code Freed\n");

  return exitCode;
}