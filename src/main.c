#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "tokens.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"
#include "generic.h"
#include "scope.h"
#include "eval.h"

// (print args...)
// prints given arguments
Generic StdLib_print(Scope *p_scope, Generic args[], int length, int lineNumber) {
  for (int i = 0; i < length; i++) {
    Generic_print(args[i]);
  }

  return Generic_new(TYPE_VOID, NULL);
}

// (is a b)
// checks for equality between a and b
// returns 1 for true, 0 for false
Generic StdLib_is(Scope *p_scope, Generic args[], int length, int lineNumber) {
  // error handling for incorrect number of args
  if (length < 2) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied less arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  } else if (length > 2) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied more arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  }

  int res = 0;

  if (args[0].type == args[1].type) {
    switch (args[0].type) {
      case TYPE_FLOAT:
        if (*((double *) args[0].p_val) == *((double *) args[1].p_val)) res = 1;
        break;
      case TYPE_INT:
        if (*((int *) args[0].p_val) == *((int *) args[1].p_val)) res = 1;
        break;
      case TYPE_STRING:
        if (strcmp(*((char **) args[0].p_val), *((char **) args[1].p_val)) == 0) res = 1;
        break;
      case TYPE_VOID:
        res = 1;
        break;
      case TYPE_NATIVEFUNCTION:
        if (args[0].p_val == args[1].p_val) res = 1;
        break;
      case TYPE_FUNCTION:
        if (args[0].p_val == args[1].p_val) res = 1;
        break;
    }
  }

  return Generic_new(TYPE_INT, &res);
}


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

  // create global scope
  Scope *p_global = Scope_new(NULL);

  // populate global scope with stdlib
  Scope_set(p_global, "print", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_print));
  Scope_set(p_global, "is", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_is));

  eval(p_headAstNode, p_global);

  return 0;
}