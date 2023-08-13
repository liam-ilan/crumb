#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "stdlib.h"
#include "generic.h"
#include "ast.h"
#include "scope.h"
#include "eval.h"
#include "list.h"

/* tools, used later in stdlib */
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

// validate type of argument
void validateType(enum Type allowedTypes[], int typeCount, enum Type type, int argNum, int lineNumber, char* funcName) {
  bool valid = false;
  
  // check if type is valid
  for (int i = 0; i < typeCount; i++) {
    if (type == allowedTypes[i]) {
      valid = true;
      break;
    }
  }

  if (!valid) {
    // print error msg
    printf("Runtime Error @ Line %i: %s function requires ", lineNumber, funcName);

    for (int i = 0; i < typeCount - 1; i++) {
      printf("%s type or ", getTypeString(allowedTypes[i]));
    }

    printf("%s type", getTypeString(allowedTypes[typeCount - 1]));
    printf(" for argument #%i, %s type supplied instead.\n", argNum, getTypeString(type));

    exit(0);
  }
}

// validate that argument is binary
void validateBinary(int *p_val, int argNum, int lineNumber, char* funcName) {
  if (*(p_val) != 0 && *(p_val) != 1) {
    printf(
      "Runtime Error @ Line %i: %s function expected 0 or 1 for argument #%i, %i supplied instead.\n", 
      lineNumber, funcName, argNum, *p_val
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
    AstNode *p_head = ((AstNode *) func->p_val)->p_headChild;

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
    Generic *res = eval(p_curr, p_local);

    // free scope and return
    Scope_free(p_local);
    return res;

  } else {
    printf(
      "Runtime Error @ Line %i: Attempted to call %s instead of function.\n", 
      lineNumber, getTypeString(func->type)
    );
    exit(0);
  }
}

/* IO */
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

/* comparissions */
// (is a b)
// checks for equality between a and b
// returns 1 for true, 0 for false
Generic *StdLib_is(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // return
  return Generic_new(TYPE_INT, Generic_is(args[0], args[1]), 0);
}

// (less_than a b)
// checks if a is less than b
Generic *StdLib_less_than(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_FLOAT, TYPE_INT};

  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "less_than");
  validateType(allowedTypes, 2, args[1]->type, 2, lineNumber, "less_than");

  // do comparision and return
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = 0;

  // handle all type cases
  if (args[0]->type == TYPE_INT && args[1]->type == TYPE_INT) {
    if (*((int *) args[0]->p_val) < *((int *) args[1]->p_val)) *p_res = 1;
  } else if (args[0]->type == TYPE_INT && args[1]->type == TYPE_FLOAT) {
    if (*((int *) args[0]->p_val) < *((double *) args[1]->p_val)) *p_res = 1;
  } else if (args[0]->type == TYPE_FLOAT && args[1]->type == TYPE_INT) {
    if (*((double *) args[0]->p_val) < *((int *) args[1]->p_val)) *p_res = 1;
  } else if (args[0]->type == TYPE_FLOAT && args[1]->type == TYPE_FLOAT) {
    if (*((double *) args[0]->p_val) < *((double *) args[1]->p_val)) *p_res = 1;
  }

  return Generic_new(TYPE_INT, p_res, 0); 
}

// (greater_than a b)
// checks if a is greater than b
Generic *StdLib_greater_than(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_FLOAT, TYPE_INT};

  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "greater_than");
  validateType(allowedTypes, 2, args[1]->type, 2, lineNumber, "greater_than");

  // do comparision and return
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = 0;

  // handle all type cases
  if (args[0]->type == TYPE_INT && args[1]->type == TYPE_INT) {
    if (*((int *) args[0]->p_val) > *((int *) args[1]->p_val)) *p_res = 1;
  } else if (args[0]->type == TYPE_INT && args[1]->type == TYPE_FLOAT) {
    if (*((int *) args[0]->p_val) > *((double *) args[1]->p_val)) *p_res = 1;
  } else if (args[0]->type == TYPE_FLOAT && args[1]->type == TYPE_INT) {
    if (*((double *) args[0]->p_val) > *((int *) args[1]->p_val)) *p_res = 1;
  } else if (args[0]->type == TYPE_FLOAT && args[1]->type == TYPE_FLOAT) {
    if (*((double *) args[0]->p_val) > *((double *) args[1]->p_val)) *p_res = 1;
  }

  return Generic_new(TYPE_INT, p_res, 0); 
}

/* logical operators */
// (not a)
// returns 1 if a = 0, 0 if a = 1
Generic *StdLib_not(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  enum Type allowedTypes[] = {TYPE_INT};
  validateType(allowedTypes, 1, args[0]->type, 1, lineNumber, "not");

  validateBinary(args[0]->p_val, 1, lineNumber, "not");

  int *p_res = (int *) malloc(sizeof(int));
  *p_res = 1 - *((int *) args[0]->p_val);

  return Generic_new(TYPE_INT, p_res, 0);
}

// (and a b)
// returns a and b
Generic *StdLib_and(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  enum Type allowedTypes[] = {TYPE_INT};
  validateType(allowedTypes, 1, args[0]->type, 1, lineNumber, "and");
  validateType(allowedTypes, 1, args[1]->type, 2, lineNumber, "and");

  validateBinary(args[0]->p_val, 1, lineNumber, "and");
  validateBinary(args[1]->p_val, 2, lineNumber, "and");

  int *p_res = (int *) malloc(sizeof(int));
  *p_res = *((int *) args[0]->p_val) && *((int *) args[1]->p_val);

  return Generic_new(TYPE_INT, p_res, 0);
}

// (or a b)
// returns a or b
Generic *StdLib_or(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  enum Type allowedTypes[] = {TYPE_INT};
  validateType(allowedTypes, 1, args[0]->type, 1, lineNumber, "or");
  validateType(allowedTypes, 1, args[1]->type, 2, lineNumber, "or");

  validateBinary(args[0]->p_val, 1, lineNumber, "or");
  validateBinary(args[1]->p_val, 2, lineNumber, "or");

  int *p_res = (int *) malloc(sizeof(int));
  *p_res = *((int *) args[0]->p_val) || *((int *) args[1]->p_val);

  return Generic_new(TYPE_INT, p_res, 0);
}

/* arithmetic */
// (add arg1 arg2 arg3 ...)
// sums all args
Generic *StdLib_add(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, (int) INFINITY, length, lineNumber);

  // flag if we can return integer, or if we must return float
  bool resIsInt = true;

  // type check
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  for (int i = 0; i < length; i++) {
    resIsInt = args[i]->type == TYPE_INT && resIsInt;
    validateType(allowedTypes, 2, args[i]->type, i + 1, lineNumber, "add");
  };
  
  if (resIsInt) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = 0;

    // add each arg to *p_res
    for (int i = 0; i < length; i++) {
      *p_res += *((int *) args[i]->p_val);
    }

    return Generic_new(TYPE_INT, p_res, 0);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));
    *p_res = 0;

    // add each arg to *p_res
    for (int i = 0; i < length; i++) {
      *p_res += args[i]->type == TYPE_FLOAT 
        ? *((double *) args[i]->p_val) 
        : *((int *) args[i]->p_val);
    }

    return Generic_new(TYPE_FLOAT, p_res, 0);
  }
}

// (subtract arg1 arg2 arg3 ...)
// returns arg1 - arg2 - arg3 - ...
Generic *StdLib_subtract(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, (int) INFINITY, length, lineNumber);

  // flag if we can return integer, or if we must return float
  bool resIsInt = true;

  // type check
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  for (int i = 0; i < length; i++) {
    resIsInt = args[i]->type == TYPE_INT && resIsInt;
    validateType(allowedTypes, 2, args[i]->type, i + 1, lineNumber, "subtract");
  };
  
  if (resIsInt) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0]->p_val);

    // subtract each arg from *p_res
    for (int i = 1; i < length; i++) {
      *p_res -= *((int *) args[i]->p_val);
    }

    return Generic_new(TYPE_INT, p_res, 0);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));
    *p_res = args[0]->type == TYPE_FLOAT 
        ? *((double *) args[0]->p_val) 
        : *((int *) args[0]->p_val);;

    // subtract each arg from *p_res
    for (int i = 1; i < length; i++) {
      *p_res -= args[i]->type == TYPE_FLOAT 
        ? *((double *) args[i]->p_val) 
        : *((int *) args[i]->p_val);
    }

    return Generic_new(TYPE_FLOAT, p_res, 0);
  }
}

// (divide arg1 arg2 arg3 ...)
// returns arg1 / arg2 / arg3 / ...
Generic *StdLib_divide(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, (int) INFINITY, length, lineNumber);

  // type check
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  for (int i = 0; i < length; i++) {
    validateType(allowedTypes, 2, args[i]->type, i + 1, lineNumber, "divide");
  };
  
  // initial value
  double *p_res = (double *) malloc(sizeof(double));
  *p_res = args[0]->type == TYPE_FLOAT 
      ? *((double *) args[0]->p_val) 
      : *((int *) args[0]->p_val);;

  // divide each arg from *p_res
  for (int i = 1; i < length; i++) {
    double val = args[i]->type == TYPE_FLOAT 
      ? *((double *) args[i]->p_val) 
      : *((int *) args[i]->p_val);

    if (val == 0) {
      // throw error for division by 0
      printf(
        "Runtime Error @ Line %i: Division by 0.\n", 
        lineNumber
      );
      exit(0);
    };

    *p_res /= val;
  }

  return Generic_new(TYPE_FLOAT, p_res, 0);
}

// (multiply arg1 arg2 arg3 ...)
// returns arg1 * arg2 * arg3 * ...
Generic *StdLib_multiply(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, (int) INFINITY, length, lineNumber);

  // flag if we can return integer, or if we must return float
  bool resIsInt = true;

  // type check
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  for (int i = 0; i < length; i++) {
    resIsInt = args[i]->type == TYPE_INT && resIsInt;
    validateType(allowedTypes, 2, args[i]->type, i + 1, lineNumber, "multiply");
  };
  
  if (resIsInt) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0]->p_val);;

    // multiply each arg to *p_res
    for (int i = 1; i < length; i++) {
      *p_res *= *((int *) args[i]->p_val);
    }

    return Generic_new(TYPE_INT, p_res, 0);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));
    *p_res = args[0]->type == TYPE_FLOAT 
        ? *((double *) args[0]->p_val) 
        : *((int *) args[0]->p_val);

    // multiply each arg to *p_res
    for (int i = 1; i < length; i++) {
      *p_res *= args[i]->type == TYPE_FLOAT 
        ? *((double *) args[i]->p_val) 
        : *((int *) args[i]->p_val);
    }

    return Generic_new(TYPE_FLOAT, p_res, 0);
  }
}

// (remainder a b)
// returns the a remainder b
Generic *StdLib_remainder(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_INT};
  validateType(allowedTypes, 1, args[0]->type, 1, lineNumber, "remainder");
  validateType(allowedTypes, 1, args[1]->type, 2, lineNumber, "remainder");

  if (*((int *) args[1]->p_val) == 0) {
    // throw error for division by 0
    printf(
      "Runtime Error @ Line %i: Remainder of division by 0.\n", 
      lineNumber
    );
    exit(0);
  };

  // do modulus and return
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = *((int *) args[0]->p_val) % *((int *) args[1]->p_val);
  return Generic_new(TYPE_INT, p_res, 0);
}

// (negative a)
// returns -a
Generic *StdLib_negative(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "negative");
  
  if (args[0]->type == TYPE_INT) {
    // int case
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = -(*((int *) args[0]->p_val));
    return Generic_new(TYPE_INT, p_res, 0);
  } else {
    // float case
    double *p_res = (double *) malloc(sizeof(double));
    *p_res = -(*((double *) args[0]->p_val));
    return Generic_new(TYPE_FLOAT, p_res, 0);
  }
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

// (list args...)
// returns a new list with args as values
Generic *StdLib_list(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  return Generic_new(TYPE_LIST, List_new(args, length), 0);
}

// (get list index)
// returns item from list
Generic *StdLib_get(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // type check
  if (args[0]->type != TYPE_LIST) {
    printf(
      "Runtime Error @ Line %i: get function requires list for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_INT) {
    printf(
      "Runtime Error @ Line %i: get function requires int for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }

  return List_get((List *) (args[0]->p_val), *((int *) args[1]->p_val));
}

// (put list item index)
// returns list, modified with item at index
Generic *StdLib_put(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(3, 3, length, lineNumber);

  if (args[0]->type != TYPE_LIST) {
    printf(
      "Runtime Error @ Line %i: put function requires list for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[2]->type != TYPE_INT) {
    printf(
      "Runtime Error @ Line %i: put function requires int for it's 3rd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[2]->type)
    );
    exit(0);
  }

  return Generic_new(TYPE_LIST, List_put((List *) (args[0]->p_val), args[1], *((int *) args[2]->p_val)), 0);
}

// (delete list index)
// deletes item from list and returns
Generic *StdLib_delete(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // type check
  if (args[0]->type != TYPE_LIST) {
    printf(
      "Runtime Error @ Line %i: delete function requires list for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0]->type)
    );
    exit(0);
  } else if (args[1]->type != TYPE_INT) {
    printf(
      "Runtime Error @ Line %i: delete function requires int for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1]->type)
    );
    exit(0);
  }

  return Generic_new(TYPE_LIST, List_delete((List *) (args[0]->p_val), *((int *) args[1]->p_val)), 0);
}

// creates a new global scope
Scope *newGlobal(int argc, char *argv[]) {
  // create global scope
  Scope *p_global = Scope_new(NULL);

  Generic *args[argc];

  // for each arg
  for (int i = 0; i < argc; i++) {
    char **p_val = (char **) malloc(sizeof(char *));

    // create string
    char *val = (char *) malloc(sizeof(char) * (strlen(argv[i]) + 1));
    strcpy(val, argv[i]);
    *p_val = val;

    // add to args
    args[i] = Generic_new(TYPE_STRING, p_val, 0);
  }

  int *p_argsCount = (int *) malloc(sizeof(int));
  *p_argsCount = argc;

  // add arguments and arguments count
  Scope_set(p_global, "arguments", Generic_new(TYPE_LIST, List_new(args, argc), 0));
  Scope_set(p_global, "arguments_count", Generic_new(TYPE_INT, p_argsCount, 0));

  // free arguments memory (as List_new does a copy)
  for (int i = 0; i < argc; i++) {
    Generic_free(args[i]);
    args[i] = NULL;
  }

  // populate global scope with stdlib  
  /* IO */
  Scope_set(p_global, "print", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_print, 0));

  /* comparisions */
  Scope_set(p_global, "is", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_is, 0));
  Scope_set(p_global, "less_than", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_less_than, 0));
  Scope_set(p_global, "greater_than", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_greater_than, 0));

  /* logical operators */
  Scope_set(p_global, "not", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_not, 0));
  Scope_set(p_global, "and", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_and, 0));
  Scope_set(p_global, "or", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_or, 0));

  /* arithmetic */
  Scope_set(p_global, "add", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_add, 0));
  Scope_set(p_global, "subtract", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_subtract, 0));
  Scope_set(p_global, "divide", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_divide, 0));
  Scope_set(p_global, "multiply", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_multiply, 0));
  Scope_set(p_global, "remainder", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_remainder, 0));
  Scope_set(p_global, "negative", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_negative, 0));

  Scope_set(p_global, "loop", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_loop, 0));
  Scope_set(p_global, "if", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_if, 0));


  Scope_set(p_global, "read_file", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_readFile, 0));
  Scope_set(p_global, "write_file", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_writeFile, 0));
  Scope_set(p_global, "int", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_int, 0));
  Scope_set(p_global, "join", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_join, 0));
  Scope_set(p_global, "str", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_str, 0));
  Scope_set(p_global, "list", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_list, 0));
  Scope_set(p_global, "get", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_get, 0));
  Scope_set(p_global, "put", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_put, 0));
  Scope_set(p_global, "delete", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_delete, 0));

  return p_global;
}