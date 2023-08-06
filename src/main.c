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

  Token headToken = {NULL, START, NULL, 1};
  int tokenCount = lex(&headToken, code, fileLength);

  // print tokens
  Token_print(&headToken, tokenCount);
  printf("Token Count: %i\n", tokenCount);

  /* parse */
  printf("\nAST\n");

  // parse
  AstNode *p_headAstNode = parseProgram(&headToken, tokenCount);

  // print AST
  AstNode_print(p_headAstNode, 0);

  /* evaluate */
  printf("\nEVAL\n");

  eval(p_headAstNode, newGlobal());

  return 0;
}