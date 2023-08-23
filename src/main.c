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

void exitHandler() {  
  exit(0);
}

int main(int argc, char *argv[]) {

  char *codePath;

  if (argc < 2) {
    printf("Error: Supply file path to read from.\n");
    return 0;
  }

  codePath = argv[1];

  // handle debug mode
  bool debug = false;
  if (strcmp(argv[1], "-d") == 0) {
    debug = true;

    if (argc < 3) {
      printf("Error: Supply file path to read from.\n");
      return 0;
    }
    
    codePath = argv[2];
  }

  /* read file */
  if (debug) printf("\nCODE\n");

  // open file
  FILE *p_file = fopen(codePath, "r");

  if (p_file == NULL) {
    printf("Error: Could not read %s.\n", codePath);
    return 0;
  }

  // go to end, and record position (this will be the length of the file)
  fseek(p_file, 0, SEEK_END);
  long fileLength = ftell(p_file);

  // rewind to start
  rewind(p_file);

  // allocate memory (+1 for 0 terminated string)
  char *code = malloc(fileLength + 1);

  // read file and close
  fread(code, fileLength, 1, p_file);
  fclose(p_file);

  // set terminator to 0
  code[fileLength] = 0;
  
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
  atexit(exitEvents);
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

  return exitCode;
}