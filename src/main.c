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

Generic applyFunc(Generic func, Scope *p_scope, Generic args[], int length, int lineNumber) {
  if(func.type == TYPE_NATIVEFUNCTION) {
    // native func case, simply obtain cb and run
    Generic (*cb)(Scope *, Generic[], int, int) = func.p_val;
    return cb(p_scope, args, length, lineNumber);

  } else if (func.type == TYPE_FUNCTION) {
    // non-native func case
    // get ast node at head of function
    AstNode *p_head = ((AstNode *) func.p_val)->p_headChild;

    // create local scope
    Scope *p_local = Scope_new(p_scope);

    // loop until statement found, and populate local scope
    AstNode *p_curr = p_head;
    int i = 0;

    while (p_curr->opcode != OP_STATEMENT && i < length) {
      Scope_set(p_local, p_curr->val, args[i]);
      p_curr = p_curr->p_next;
      i++;
    }

    // error handling
    if (i != length) {
      // supplied too many args, throw error
      printf(
        "Runtime Error @ Line %i: Supplied more arguments than required to function.\n", 
        p_curr->lineNumber
      );
      exit(0);
    } else if (p_curr->opcode != OP_STATEMENT) {
      // supplied too little args, throw error
      printf(
        "Runtime Error @ Line %i: Supplied less arguments than required to function.\n", 
        p_head->lineNumber
      );
      exit(0);
    }

    // run statement with local scope
    Generic res = eval(p_curr, p_local);

    // free scope and return
    Scope_free(p_local);
    return res;

  } else {
    printf(
      "Runtime Error @ Line %i: Attempted to call %s instead of function.\n", 
      lineNumber, getTypeString(func.type)
    );
    exit(0);
  }
}

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

  int *p_res = (int *) malloc(sizeof(int));

  if (args[0].type == args[1].type) {
    switch (args[0].type) {
      case TYPE_FLOAT:
        if (*((double *) args[0].p_val) == *((double *) args[1].p_val)) *p_res = 1;
        break;
      case TYPE_INT:
        if (*((int *) args[0].p_val) == *((int *) args[1].p_val)) *p_res = 1;
        break;
      case TYPE_STRING:
        if (strcmp(*((char **) args[0].p_val), *((char **) args[1].p_val)) == 0) *p_res = 1;
        break;
      case TYPE_VOID:
        *p_res = 1;
        break;
      case TYPE_NATIVEFUNCTION:
        if (args[0].p_val == args[1].p_val) *p_res = 1;
        break;
      case TYPE_FUNCTION:
        if (args[0].p_val == args[1].p_val) *p_res = 1;
        break;
    }
  }

  return Generic_new(TYPE_INT, p_res);
}

// (apply f args...)
// applys args to f
Generic StdLib_apply(Scope *p_scope, Generic args[], int length, int lineNumber) {
  Generic *newArgs = &(args[1]); 
  applyFunc(args[0], p_scope, newArgs, length - 1, lineNumber);
}

// (loop i f)
// applys f, i times, passing the current index to f
Generic StdLib_loop(Scope *p_scope, Generic args[], int length, int lineNumber) {
  // error handling for incorrect number of args
  if (length > 2) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied more arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  } else if (length < 2) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied less arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  }

  // error handling to check that 2nd arg is a function, and 1st arg is an int
  if (args[0].type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: loop function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  } else if (args[1].type != TYPE_FUNCTION && args[1].type != TYPE_NATIVEFUNCTION) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: loop function requires function or native function type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1].type)
    );
    exit(0);
  }

  for (int i = 0; i < *((int *) args[0].p_val); i++) {
    int *p_arg = (int *) malloc(sizeof(int));
    *p_arg = i;
    Generic newArgs[1] = {Generic_new(TYPE_INT, p_arg)};
    applyFunc(args[1], p_scope, newArgs, 1, lineNumber);
  }

  return Generic_new(TYPE_VOID, NULL);
}

// (if c f g*)
// applys f if c == 1
// applys g or nothing if c == 0
Generic StdLib_if(Scope *p_scope, Generic args[], int length, int lineNumber) {
  // error handling for incorrect number of args
  if (length > 3) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied more arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  } else if (length < 2) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied less arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  }

  // error handling to check that 1st arg is an int, and 2nd arg is a function
  if (args[0].type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: if function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  } else if (args[1].type != TYPE_FUNCTION && args[1].type != TYPE_NATIVEFUNCTION) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: if function requires function or native function type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1].type)
    );
    exit(0);
  } else if (length == 3 && args[2].type != TYPE_FUNCTION && args[2].type != TYPE_NATIVEFUNCTION) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: if function requires function or native function type for it's 3rd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[2].type)
    );
    exit(0);
  }

  if (*((int *) args[0].p_val) == 1) applyFunc(args[1], p_scope, NULL, 0, lineNumber);
  else if (length == 3 && *((int *) args[0].p_val) == 0) applyFunc(args[2], p_scope, NULL, 0, lineNumber);
  else if (*((int *) args[0].p_val) != 0) {
    // c is not 1 or 0, throw err
    printf(
      "Runtime Error @ Line %i: if function expected 0 or 1 for it's 1st argument, %i supplied instead.\n", 
      lineNumber, *((int *) args[0].p_val)
    );
    exit(0);
  }

  return Generic_new(TYPE_VOID, NULL);
}

// (% a b)
// returns the mod of a and b
Generic StdLib_mod(Scope *p_scope, Generic args[], int length, int lineNumber) {
  // check that both args are ints
  // error handling for incorrect number of args
  if (length > 2) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied more arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  } else if (length < 2) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied less arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  }

  // error handling to check that 1st arg is an int, and 2nd arg is an int
  if (args[0].type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: %% function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  } else if (args[1].type != TYPE_INT) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: %% function requires int type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1].type)
    );
    exit(0);
  }

  // do modulus and return
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = *((int *) args[0].p_val) % *((int *) args[1].p_val);
  return Generic_new(TYPE_INT, p_res);
}

// (not a)
// returns 1 if a = 0, 0 if a = 1
Generic StdLib_not(Scope *p_scope, Generic args[], int length, int lineNumber) {
  // check that both args are ints
  // error handling for incorrect number of args
  if (length > 1) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied more arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  } else if (length < 1) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied less arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  }

  // type check
    // error handling to check that 1st arg is an int, and 2nd arg is an int
  if (args[0].type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: not function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  }

  // a is not 1 or 0, throw err
  if (*((int *) args[0].p_val) != 0 && *((int *) args[0].p_val) != 1) {
    printf(
      "Runtime Error @ Line %i: not function expected 0 or 1 for it's 1st argument, %i supplied instead.\n", 
      lineNumber, *((int *) args[0].p_val)
    );
    exit(0);
  }

  int *p_res = (int *) malloc(sizeof(int));
  *p_res = 1 - *((int *) args[0].p_val);

  
  return Generic_new(TYPE_INT, p_res);
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
  Scope_set(p_global, "apply", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_apply));
  Scope_set(p_global, "loop", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_loop));
  Scope_set(p_global, "if", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_if));
  Scope_set(p_global, "%", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_mod));
  Scope_set(p_global, "!", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_not));

  eval(p_headAstNode, p_global);

  return 0;
}