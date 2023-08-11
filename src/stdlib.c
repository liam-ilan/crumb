#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "generic.h"
#include "ast.h"
#include "scope.h"
#include "eval.h"
#include "function.h"

// validate number of arguments
void validateArgCount(int min, int max, int length, int lineNumber) {
  if (length > max) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied more arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  } else if (length < min) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: Supplied less arguments than required to function.\n", 
      lineNumber
    );
    exit(0);
  }
}

// applys a func, given arguments
// used for callbacks from the standard library
Generic *applyFunc(Generic *func, Scope *p_scope, Generic *args[], int length, int lineNumber) {
  if(func->type == TYPE_NATIVEFUNCTION) {
    // native func case, simply obtain cb and run
    Generic *(*cb)(Scope *, Generic *[], int, int) = func->p_val;

    // increase ref count
    for (int i = 0; i < length; i++) {
      args[i]->refCount++;
    }

    Generic *res = cb(p_scope, args, length, lineNumber);

    // drop ref count, and free if count is 0
    for (int i = 0; i < length; i++) {
      args[i]->refCount--;
      if (args[i]->refCount == 0) Generic_free(args[i]);
    }

    return res;

  } else if (func->type == TYPE_FUNCTION) {
    // non-native func case
    // get ast node at head of function
    AstNode *p_head = (((Function *) func->p_val)->p_ast)->p_headChild;

    // create local scope
    Scope *p_funcScope = Scope_copy(((Function *) func->p_val)->p_scope);
    p_funcScope->p_parent = p_scope;
    Scope *p_local = Scope_new(p_funcScope);

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
    Generic *res = eval(p_curr, p_local);

    // free scope and return
    Scope_free(p_local);
    Scope_free(p_funcScope);
    return res;

  } else {
    printf(
      "Runtime Error @ Line %i: Attempted to call %s instead of function.\n", 
      lineNumber, getTypeString(func->type)
    );
    exit(0);
  }
}

// (print args...)
// prints given arguments
Generic *StdLib_print(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  for (int i = 0; i < length; i++) {
    Generic_print(args[i]);
    if (i < length - 1) printf(" ");
  }

  printf("\n");

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (is a b)
// checks for equality between a and b
// returns 1 for true, 0 for false
Generic *StdLib_is(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // create res
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = 0;

  // check types
  if (args[0]->type == args[1]->type) {

    // do type conversions and check data
    switch (args[0]->type) {
      case TYPE_FLOAT:
        if (*((double *) args[0]->p_val) == *((double *) args[1]->p_val)) *p_res = 1;
        break;
      case TYPE_INT:
        if (*((int *) args[0]->p_val) == *((int *) args[1]->p_val)) *p_res = 1;
        break;
      case TYPE_STRING:
        if (strcmp(*((char **) args[0]->p_val), *((char **) args[1]->p_val)) == 0) *p_res = 1;
        break;
      case TYPE_VOID:
        *p_res = 1;
        break;
      case TYPE_NATIVEFUNCTION:
        if (args[0]->p_val == args[1]->p_val) *p_res = 1;
        break;
      case TYPE_FUNCTION:
        if (args[0]->p_val == args[1]->p_val) *p_res = 1;
        break;
    }
  }

  // return
  return Generic_new(TYPE_INT, p_res, 0);
}

// (apply f args...)
// applys args to f
Generic *StdLib_apply(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  Generic **newArgs = &(args[1]); 
  return applyFunc(args[0], p_scope, newArgs, length - 1, lineNumber);
}

// (loop n f)
// applys f, n times, passing the current index to f
// returns whatever f returns if not void
// will go to completion and return void if void is all f returns
Generic *StdLib_loop(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // error handling to check that 2nd arg is a function, and 1st arg is an int
  if (args[0]->type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: loop function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_FUNCTION && args[1]->type != TYPE_NATIVEFUNCTION) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: loop function requires function or native function type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }

  // loop
  for (int i = 0; i < *((int *) args[0]->p_val); i++) {

    // get arg to pass to cb
    int *p_arg = (int *) malloc(sizeof(int));
    *p_arg = i;
    Generic *newArgs[1] = {Generic_new(TYPE_INT, p_arg, 0)};
    
    // call cb
    Generic *res = applyFunc(args[1], p_scope, newArgs, 1, lineNumber);

    // if void type returned, free
    if (res->type != TYPE_VOID) return res;
    else Generic_free(res);
  }

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (if c f g*)
// applys f if c == 1
// applys g or nothing if c == 0
Generic *StdLib_if(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 3, length, lineNumber);

  // error handling to check that 1st arg is an int, and 2nd arg is a function
  if (args[0]->type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: if function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_FUNCTION && args[1]->type != TYPE_NATIVEFUNCTION) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: if function requires function or native function type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  } else if (length == 3 && args[2]->type != TYPE_FUNCTION && args[2]->type != TYPE_NATIVEFUNCTION) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: if function requires function or native function type for it's 3rd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[2]->type)
    );
    exit(0);
  }

  if (*((int *) args[0]->p_val) == 1) return applyFunc(args[1], p_scope, NULL, 0, lineNumber);
  else if (length == 3 && *((int *) args[0]->p_val) == 0) return applyFunc(args[2], p_scope, NULL, 0, lineNumber);
  else if (*((int *) args[0]->p_val) != 0) {
    // c is not 1 or 0, throw err
    printf(
      "Runtime Error @ Line %i: if function expected 0 or 1 for it's 1st argument, %i supplied instead.\n", 
      lineNumber, *((int *) args[0]->p_val)
    );
    exit(0);
  }

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (% a b)
// returns the mod of a and b
Generic *StdLib_mod(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // error handling to check that 1st arg is an int, and 2nd arg is an int
  if (args[0]->type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: %% function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_INT) {
    // supplied too little args, throw error
    printf(
      "Runtime Error @ Line %i: %% function requires int type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }

  // do modulus and return
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = *((int *) args[0]->p_val) % *((int *) args[1]->p_val);
  return Generic_new(TYPE_INT, p_res, 0);
}

// (! a)
// returns 1 if a = 0, 0 if a = 1
Generic *StdLib_not(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  // type check
    // error handling to check that 1st arg is an int, and 2nd arg is an int
  if (args[0]->type != TYPE_INT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: not function requires int type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  }

  // a is not 1 or 0, throw err
  if (*((int *) args[0]->p_val) != 0 && *((int *) args[0]->p_val) != 1) {
    printf(
      "Runtime Error @ Line %i: not function expected 0 or 1 for it's 1st argument, %i supplied instead.\n", 
      lineNumber, *((int *) args[0]->p_val)
    );
    exit(0);
  }

  int *p_res = (int *) malloc(sizeof(int));
  *p_res = 1 - *((int *) args[0]->p_val);

  
  return Generic_new(TYPE_INT, p_res, 0);
}

// (+ a b)
// returns a + b
Generic *StdLib_add(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // error handling for types
  if (args[0]->type != TYPE_INT && args[0]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: + function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_INT && args[1]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: + function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }
  
  if (args[0]->type == TYPE_INT && args[1]->type == TYPE_INT) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0]->p_val) + *((int *) args[1]->p_val);
    return Generic_new(TYPE_INT, p_res, 0);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types

    *p_res = (args[0]->type == TYPE_FLOAT ? *((double *) args[0]->p_val) : *((int *) args[0]->p_val))
      + (args[1]->type == TYPE_FLOAT ? *((double *) args[1]->p_val) : *((int *) args[1]->p_val));

    return Generic_new(TYPE_FLOAT, p_res, 0); 
  }
}

// (- a b)
// returns a - b
Generic *StdLib_sub(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // error handling for types
  if (args[0]->type != TYPE_INT && args[0]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: - function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_INT && args[1]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: - function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }
  
  if (args[0]->type == TYPE_INT && args[1]->type == TYPE_INT) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0]->p_val) - *((int *) args[1]->p_val);
    return Generic_new(TYPE_INT, p_res, 0);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types

    *p_res = (args[0]->type == TYPE_FLOAT ? *((double *) args[0]->p_val) : *((int *) args[0]->p_val))
      - (args[1]->type == TYPE_FLOAT ? *((double *) args[1]->p_val) : *((int *) args[1]->p_val));

    return Generic_new(TYPE_FLOAT, p_res, 0); 
  }
}

// (* a b)
// returns a * b
Generic *StdLib_mult(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // error handling for types
  if (args[0]->type != TYPE_INT && args[0]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: * function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_INT && args[1]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: * function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }
  
  if (args[0]->type == TYPE_INT && args[1]->type == TYPE_INT) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0]->p_val) * *((int *) args[1]->p_val);
    return Generic_new(TYPE_INT, p_res, 0);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types

    *p_res = (args[0]->type == TYPE_FLOAT ? *((double *) args[0]->p_val) : *((int *) args[0]->p_val))
      * (args[1]->type == TYPE_FLOAT ? *((double *) args[1]->p_val) : *((int *) args[1]->p_val));

    return Generic_new(TYPE_FLOAT, p_res, 0); 
  }
}

// (/ a b)
// returns a / b
Generic *StdLib_div(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // error handling for types
  if (args[0]->type != TYPE_INT && args[0]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: / function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_INT && args[1]->type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: / function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }
  
  if (
    args[0]->type == TYPE_INT && args[1]->type == TYPE_INT 
    && *((int *) args[0]->p_val) % *((int *) args[1]->p_val) == 0
  ) {
    // case where we can return int (a is divisible by b and a an b are ints)
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0]->p_val) / *((int *) args[1]->p_val);
    return Generic_new(TYPE_INT, p_res, 0);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types
    *p_res = (args[0]->type == TYPE_FLOAT ? *((double *) args[0]->p_val) : *((int *) args[0]->p_val))
      / (args[1]->type == TYPE_FLOAT ? *((double *) args[1]->p_val) : *((int *) args[1]->p_val));

    return Generic_new(TYPE_FLOAT, p_res, 0); 
  }
}

// (read_file filepath)
// reads text at filepath and returns
Generic *StdLib_readFile(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);
  
  // check that type of arg is string
  if (args[0]->type != TYPE_STRING) {
    printf(
      "Runtime Error @ Line %i: read_file function requires string type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  }

  FILE *p_file = fopen(*((char ** )args[0]->p_val), "r");

  // check the file succesfully opened
  if (p_file == NULL) {
    printf(
      "Runtime Error @ Line %i: Cannot read file %s.\n", 
      lineNumber, *((char ** )args[0]->p_val)
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

  // set terminator to 0
  res[fileLength] = 0;

  char **p_res = (char **) malloc(sizeof(char *));
  *p_res = res;

  // return
  return Generic_new(TYPE_STRING, p_res, 0);
}

// (write_file filepath string)
// writes string to filepath
Generic *StdLib_writeFile(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);
  
  // check that type of args is string
  if (args[0]->type != TYPE_STRING) {
    printf(
      "Runtime Error @ Line %i: write_file function requires string type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_STRING) {
    printf(
      "Runtime Error @ Line %i: write_file function requires string type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }

  FILE *p_file = fopen(*((char ** )args[0]->p_val), "w+");

  // check the file succesfully opened
  if (p_file == NULL) {
    printf(
      "Runtime Error @ Line %i: Cannot write file %s.\n", 
      lineNumber, *((char ** )args[0]->p_val)
    );
    exit(0); 
  }

  fprintf(p_file, "%s\n", *((char ** )args[1]->p_val));
  fclose(p_file);
  return Generic_new(TYPE_VOID, NULL, 0);
}

// (int x)
// returns x as an integer, if string, float, or int passed
Generic *StdLib_int(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  // make sure first type is string
  // check that type of arg is string or int or float
  if (args[0]->type != TYPE_STRING && args[0]->type != TYPE_FLOAT && args[0]->type != TYPE_INT) {
    printf(
      "Runtime Error @ Line %i: int function requires string or int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  }

  int *p_res = malloc(sizeof(int));

  if (args[0]->type == TYPE_STRING) {
    char *str = *((char **) args[0]->p_val);
    *p_res = atoi(str);
  } else if (args[0]->type == TYPE_FLOAT) {
    double f = *((double *) args[0]->p_val);
    *p_res = (int) f;
  } else {
    int i = *((int *) args[0]->p_val);
    *p_res = i;
  }

  return Generic_new(TYPE_INT, p_res, 0);
}

// (str x)
// returns x as a string, if string, float, or int passed
Generic *StdLib_str(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  // type check
  if (args[0]->type != TYPE_STRING && args[0]->type != TYPE_FLOAT && args[0]->type != TYPE_INT) {
    printf(
      "Runtime Error @ Line %i: str function requires string or int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  }
  
  char *res;

  if (args[0]->type == TYPE_FLOAT) {
    int length = snprintf(NULL, 0, "%f", *((double *) args[0]->p_val)); // get length
    res = malloc(sizeof(char) * (length + 1)); // allocate memory
    snprintf(res, length + 1, "%f", *((double *) args[0]->p_val)); // populate memory
  } else if (args[0]->type == TYPE_INT) {
    int length = snprintf(NULL, 0, "%i", *((int *) args[0]->p_val));
    res = malloc(sizeof(char) * (length + 1));
    snprintf(res, length + 1, "%i", *((int *) args[0]->p_val));
  } else {
    int length = snprintf(NULL, 0, "%s", *((char **) args[0]->p_val));
    res = malloc(sizeof(char) * (length + 1));
    snprintf(res, length + 1, "%s", *((char **) args[0]->p_val));
  }

  char **p_res = (char **) malloc(sizeof(char *));
  *p_res = res;

  return Generic_new(TYPE_STRING, p_res, 0);
}

// (join str1 str2)
// joins strings together, and returns a new string
Generic *StdLib_join(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // type checking
  if (args[0]->type != TYPE_STRING) {
    printf(
      "Runtime Error @ Line %i: join function requires string for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_STRING) {
    printf(
      "Runtime Error @ Line %i: join function requires string for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }

  char *res = (char *) malloc(
    sizeof(char) * 
    (
      strlen(*((char **) args[0]->p_val))
      + strlen(*((char **) args[1]->p_val))
      + 1
    )
  );

  strcpy(res, *((char **) args[0]->p_val));
  strcat(res, *((char **) args[1]->p_val));

  char **p_res = (char **) malloc(sizeof(char *));
  *p_res = res;

  return Generic_new(TYPE_STRING, p_res, 0);
}

// creates a new global scope
Scope *newGlobal() {
  // create global scope
  Scope *p_global = Scope_new(NULL);

  // populate global scope with stdlib
  Scope_set(p_global, "print", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_print, 0));
  Scope_set(p_global, "is", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_is, 0));
  Scope_set(p_global, "apply", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_apply, 0));
  Scope_set(p_global, "loop", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_loop, 0));
  Scope_set(p_global, "if", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_if, 0));
  Scope_set(p_global, "%", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_mod, 0));
  Scope_set(p_global, "!", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_not, 0));
  Scope_set(p_global, "+", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_add, 0));
  Scope_set(p_global, "-", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_sub, 0));
  Scope_set(p_global, "*", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_mult, 0));
  Scope_set(p_global, "/", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_div, 0));
  Scope_set(p_global, "read_file", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_readFile, 0));
  Scope_set(p_global, "write_file", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_writeFile, 0));
  Scope_set(p_global, "int", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_int, 0));
  Scope_set(p_global, "join", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_join, 0));
  Scope_set(p_global, "str", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_str, 0));

  return p_global;
}