#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

#include "tokens.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"
#include "generic.h"
#include "scope.h"
#include "eval.h"
#include "stdlib.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Error: Supply file path to read from.\n");
    return 0;
  }

  // open file
  FILE *p_file = fopen(argv[1], "r");

  if (p_file == NULL) {
    printf("Error: Could not read %s.\n", argv[1]);
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

  /* code */
  printf("\nCODE\n");

  // print code
  printf("%s\n", code);

  /* lex */
  printf("\nTOKENS\n");

  // create initial token
  Token *p_headToken = (Token *) malloc(sizeof(Token));
  p_headToken->lineNumber = 1;
  p_headToken->type = START;
  p_headToken->val = NULL;
  p_headToken->p_next = NULL;

  // lex
  int tokenCount = lex(p_headToken, code, fileLength);

  // print tokens
  Token_print(p_headToken, tokenCount);
  printf("Token Count: %i\n", tokenCount);

  /* parse */
  printf("\nAST\n");

  // parse
  AstNode *p_headAstNode = parseProgram(p_headToken, tokenCount);

  // print AST
  AstNode_print(p_headAstNode, 0);

  /* evaluate */
  printf("\nEVAL\n");
  Scope *p_global = newGlobal();
  Generic *res = eval(p_headAstNode, p_global);

  // get exit code
  int exitCode = 0;
  if (res->type == TYPE_INT) {
    exitCode = *((int *) res->p_val);
  }

  res->refCount = 0;

  /* free */
  printf("\nFREE\n");

  // free code
  free(code);
  code = NULL;
  printf("Code Freed\n");

  // free tokens
  Token_free(p_headToken);
  p_headToken = NULL;
  printf("Tokens Freed\n");

  // free ast
  AstNode_free(p_headAstNode);
  p_headAstNode = NULL;
  printf("AST Freed\n");
  
  // free global scope
  Scope_free(p_global);
  p_global = NULL;
  printf("Global Scope Freed\n");

  // free generic for exit code
  Generic_free(res);
  res = NULL;
  printf("Exit Code Generic Freed");

  return exitCode;
}