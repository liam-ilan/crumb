#include "run.h"

#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"
#include "events.h"
#include "stdlib.h"
#include "eval.h"
#include "file.h"
#include "scope.h"

void exitHandler() {  
  exit(0);
}

int run(char *code, long length, int argc, char *argv[], bool debug) {
  if (debug) {
    printf("\nTOKENS\n");
  }

  /* lex */
  // create initial token
  Token *p_headToken = (Token *) malloc(sizeof(Token));
  p_headToken->lineNumber = 1;
  p_headToken->type = TOK_START;
  p_headToken->val = NULL;
  p_headToken->p_next = NULL;

  int tokenCount = lex(p_headToken, code, length);

  if (debug) {
    // print tokens
    Token_print(p_headToken, tokenCount);
    printf("Token Count: %i\n", tokenCount);
    printf("\nAST\n");
  }

  /* parse */
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

  Scope *p_global = newGlobal(argc, argv);
  Generic *res = eval(p_headAstNode, p_global, 0);

  // get exit code
  int exitCode = 0;
  if (res->type == TYPE_INT) {
    exitCode = *((int *) res->p_val);
  }
  res->refCount = 0;

  /* free */
  if (debug) printf("\nFREE\n");

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

  // free file cache.
  FileCache_free();
  if (debug) printf("File Cache Freed\n");

  // free return code.
  Generic_free(res);
  res = NULL;
  if (debug) printf("Return Code Freed\n");

  return exitCode;
}