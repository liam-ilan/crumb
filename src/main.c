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

#define CRUMB_VERSION ("v0.0.4")

void exitHandler() {  
  exit(0);
}

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

  // Check if stdin is empty.
  fseek(stdin, 0, SEEK_END);
  bool stdinEmpty = ftell(stdin) == 0;
  rewind(stdin);

  if (!stdinEmpty) {

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

  } else if ((debug && argc >= 3) || (!debug && argc >= 2)) {

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

  } else {
    printf("Error: Program not supplied through pipe or argument.\n");
    return 0;
  }
  
  if (debug) {
    printf("%s\n", code);
    printf("\nTOKENS\n");
  }

  /* lex */
  // create initial token
  Token *p_headToken = (Token *) malloc(sizeof(Token));
  p_headToken->lineNumber = 1;
  p_headToken->type = TOK_START;
  p_headToken->val = NULL;
  p_headToken->p_next = NULL;

  // lex
  int tokenCount = lex(p_headToken, code, fileLength);

  if (debug) {
    // print tokens
    Token_print(p_headToken, tokenCount);
    printf("Token Count: %i\n", tokenCount);
    /* parse */
    printf("\nAST\n");
  }

  // parse
  AstNode *p_headAstNode = parseProgram(p_headToken, tokenCount);
  
  if (debug) {
    // print AST
    AstNode_print(p_headAstNode, 0);
    printf("\nEVAL\n");
  };

  /* evaluate */
  initEvents();

  // cleanly handle exit events
  signal(SIGTERM, exitHandler);
  signal(SIGINT, exitHandler);

  Scope *p_global = newGlobal(argc, argv, 1 + debug);
  Generic *res = eval(p_headAstNode, p_global, 0);

  // get exit code
  int exitCode = 0;
  if (res->type == TYPE_INT) {
    exitCode = *((int *) res->p_val);
  }

  res->refCount = 0;

  /* free */
  if (debug) printf("\nFREE\n");

  // free code
  free(code);
  code = NULL;
  if (debug) printf("Code Freed\n");

  // free tokens
  Token_free(p_headToken);
  p_headToken = NULL;
  if (debug) printf("Tokens Freed\n");

  // free ast
  AstNode_free(p_headAstNode);
  p_headAstNode = NULL;
  if (debug) printf("AST Freed\n");
  
  // free global scope
  Scope_free(p_global);
  p_global = NULL;
  if (debug) printf("Global Scope Freed\n");

  // free generic for exit code
  Generic_free(res);
  res = NULL;
  if (debug) printf("Exit Code Generic Freed\n");

  FileCache_free();
  if (debug) printf("File Cache Freed\n");

  return exitCode;
}