#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "stdlib.h"
#include "generic.h"
#include "ast.h"
#include "scope.h"
#include "eval.h"
#include "list.h"
#include "events.h"
#include "file.h"
#include "lex.h"
#include "tokens.h"
#include "parse.h"

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

// verify that a bare minimum amount of args are passed
void validateMinArgCount(int min, int length, int lineNumber) {
  if (length < min) {
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

// validate that argument is within a range
void validateRange(int *p_val, int min, int max, int argNum, int lineNumber, char* funcName) {
  if (*p_val > max || *p_val < min) {
    printf(
      "Runtime Error @ Line %i: %s function expected a value in the range [%i, %i] for argument #%i, %i supplied instead.\n", 
      lineNumber, funcName, min, max, argNum, *p_val
    );

    exit(0);
  }
}

// validate that value is at least a minimum
void validateMin(int *p_val, int min, int argNum, int lineNumber, char* funcName) {
  if (*p_val < min) {
    printf(
      "Runtime Error @ Line %i: %s function expected a minimum value of %i for argument #%i, %i supplied instead.\n", 
      lineNumber, funcName, min, argNum, *p_val
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

    if (func->refCount == 0) Generic_free(func);

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
    Generic *res = eval(p_curr, p_local, 0);

    // free scope
    Scope_free(p_local);

    // free function if no references
    if (func->refCount == 0) Generic_free(func);
    
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

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (input)
// gets input from stdin
Generic *StdLib_input(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(0, 0, length, lineNumber);

  // eat through anything in the input buffer
  enableRaw();
  char eat = readChar();
  while (eat != '\0') eat = readChar();
  disableRaw();

  // enable echo
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

  // initial allocation of empty string
  char *res = (char *) malloc(sizeof(char));
  res[0] = '\0';

  // loop through every char
  char c = getchar();
  while (c != '\n') {
    // reallocate and add char
    res = realloc(res, sizeof(char) * (strlen(res) + 1 + 1));
    strncat(res, &c, 1);

    c = getchar();
  }

  // enable events again (turn off echo)
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &run_termios);

  // create pointer
  char **p_res = (char **) malloc(sizeof(char *));
  *p_res = res;

  return Generic_new(TYPE_STRING, p_res, 0);
}

// (columns)
// returns number of columns in terminal
Generic *StdLib_columns(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  // get current dimensions
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  // create pointer to rows
  int *rows = (int *) malloc(sizeof(int));
  *rows = w.ws_col;

  // return generic
  return Generic_new(TYPE_INT, rows, 0);
}

// (rows)
// returns number of rows in terminal
Generic *StdLib_rows(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  // get current dimensions
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  // create pointer to rows
  int *rows = (int *) malloc(sizeof(int));
  *rows = w.ws_row;

  // return generic
  return Generic_new(TYPE_INT, rows, 0);
}

// (read_file filepath)
// reads text at filepath and returns
Generic *StdLib_read_file(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_STRING};
  validateType(allowedTypes, 1, args[0]->type, 1, lineNumber, "read_file");

  // read file
  char *res = readFile(*((char **) args[0]->p_val));
  
  // if file couldn't be read, return void
  if (res == NULL) return Generic_new(TYPE_VOID, NULL, 0);

  // malloc pointer to string
  char **p_res = (char **) malloc(sizeof(char *));
  *p_res = res;

  // return
  return Generic_new(TYPE_STRING, p_res, 0);
}

// (write_file filepath string)
// writes string to filepath
Generic *StdLib_write_file(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);
  
  // check that type of args is string
  enum Type allowedTypes[] = {TYPE_STRING};
  validateType(allowedTypes, 1, args[0]->type, 1, lineNumber, "write_file");
  validateType(allowedTypes, 1, args[1]->type, 2, lineNumber, "write_file");

  writeFile(*((char **) args[0]->p_val), *((char **) args[1]->p_val), lineNumber);
  return Generic_new(TYPE_VOID, NULL, 0);
}

// (event)
// returns a string containing the current event
Generic *StdLib_event(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(0, 0, length, lineNumber);

  char **p_res = (char **) malloc(sizeof(char *));
  *p_res = event();
  
  return Generic_new(TYPE_STRING, p_res, 0);
}

// (use path1 path2 path3 ... fn)
// creates a new scope, evaluates the code in path1, path2, and path3, and then uses said scope to evaluate fn
Generic *StdLib_use(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateMinArgCount(2, length, lineNumber);

  // validate that all arguments with exception of last one are strings
  for (int i = 0; i < length - 1; i++) {
    enum Type allowedTypes[] = {TYPE_STRING};
    validateType(allowedTypes, 1, args[i]->type, i + 1, lineNumber, "use");
  }

  // validate that last argument is a function
  enum Type allowedTypes[] = {TYPE_FUNCTION, TYPE_NATIVEFUNCTION};
  validateType(allowedTypes, 2, args[length - 1]->type, length, lineNumber, "use");

  // create a new scope for all paths and fn to run in
  Scope *p_newScope = Scope_new(p_scope);

  // for each path
  for (int i = 0; i < length - 1; i++) {

    // read file
    char *code = readFile(*((char **) args[i]->p_val));

    // throw error if file could not be read
    if (code == NULL) {
      printf(
        "Runtime Error @ Line %i: Cannot read file \"%s\".\n", 
        lineNumber, *((char **) args[i]->p_val)
      );
      exit(0); 
    }

    // create initial token
    Token *p_headToken = (Token *) malloc(sizeof(Token));
    p_headToken->lineNumber = 1;
    p_headToken->type = TOK_START;
    p_headToken->val = NULL;
    p_headToken->p_next = NULL;

    // lex
    int tokenCount = lex(p_headToken, code, strlen(code));

    // parse
    AstNode *p_headAstNode = parseProgram(p_headToken, tokenCount);

    // eval and free
    Generic_free(eval(p_headAstNode, p_newScope, 0));

    // free memory
    // code
    free(code);
    code = NULL;

    // tokens
    Token_free(p_headToken);
    p_headToken = NULL;

    // ast
    AstNode_free(p_headAstNode);
    p_headAstNode = NULL;
  }

  // apply callback with new scope
  Generic *res = applyFunc(args[length - 1], p_newScope, NULL, 0, lineNumber);

  // free new scope and return
  Scope_free(p_newScope);
  return res;
}

/* comparissions */
// (is a b)
// checks for equality between a and b
// returns 1 for true, 0 for false
Generic *StdLib_is(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  // return
  int *res = (int *) malloc(sizeof(int));
  *res = Generic_is(args[0], args[1]);
  return Generic_new(TYPE_INT, res, 0);
}

// (less_than a b)
// checks if a is less than b
Generic *StdLib_less_than(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_FLOAT, TYPE_INT};

  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "less_than");
  validateType(allowedTypes, 2, args[1]->type, 2, lineNumber, "less_than");

  // do comparision and return
  Generic *a = args[0];
  Generic *b = args[1];
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = (
    (a->type == TYPE_FLOAT ? *((double *) a->p_val) : *((int *) a->p_val))
    < (b->type == TYPE_FLOAT ? *((double *) b->p_val) : *((int *) b->p_val))
  );

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
  Generic *a = args[0];
  Generic *b = args[1];
  int *p_res = (int *) malloc(sizeof(int));
  *p_res = (
    (a->type == TYPE_FLOAT ? *((double *) a->p_val) : *((int *) a->p_val))
    > (b->type == TYPE_FLOAT ? *((double *) b->p_val) : *((int *) b->p_val))
  );

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
  validateMinArgCount(2, length, lineNumber);

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
  validateMinArgCount(2, length, lineNumber);

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
  validateMinArgCount(2, length, lineNumber);

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
  validateMinArgCount(2, length, lineNumber);

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
  
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "remainder");
  validateType(allowedTypes, 2, args[1]->type, 2, lineNumber, "remainder");

  if ((args[1]->type == TYPE_FLOAT ? *((double *) args[1]->p_val) : *((int *) args[1]->p_val)) == 0) {
    // throw error for division by 0
    printf(
      "Runtime Error @ Line %i: Remainder of division by 0.\n", 
      lineNumber
    );
    exit(0);
  };

  if (args[0]->type == TYPE_INT && args[1]->type == TYPE_INT) {
    // if we can return integer
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0]->p_val) % *((int *) args[1]->p_val);
    return Generic_new(TYPE_INT, p_res, 0);
  } else {
    // if we must return float
    double *p_res = (double *) malloc(sizeof(double));

    *p_res = fmod(
      (args[0]->type == TYPE_FLOAT ? *((double *) args[0]->p_val) : *((int *) args[0]->p_val)),
      (args[1]->type == TYPE_FLOAT ? *((double *) args[1]->p_val) : *((int *) args[1]->p_val))
    );

    return Generic_new(TYPE_FLOAT, p_res, 0);
  }
}

// (power a b)
// returns the a to the power of b
Generic *StdLib_power(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "power");
  validateType(allowedTypes, 2, args[1]->type, 2, lineNumber, "power");

  if (args[0]->type == TYPE_INT && args[1]->type == TYPE_INT) {
    // if we can return integer
    int *p_res = (int *) malloc(sizeof(int));

    *p_res = pow(
      *((int *) args[0]->p_val),
      *((int *) args[1]->p_val)
    );

    return Generic_new(TYPE_INT, p_res, 0);
  } else {
    // if we must return float
    double *p_res = (double *) malloc(sizeof(double));

    *p_res = pow(
      (args[0]->type == TYPE_FLOAT ? *((double *) args[0]->p_val) : *((int *) args[0]->p_val)),
      (args[1]->type == TYPE_FLOAT ? *((double *) args[1]->p_val) : *((int *) args[1]->p_val))
    );

    return Generic_new(TYPE_FLOAT, p_res, 0);
  }
}

// (random)
// returns random number from 0 to 1
Generic *StdLib_random(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(0, 0, length, lineNumber);

  double *p_res = (double *) malloc(sizeof(double));
  *p_res = (double) rand() / (double) RAND_MAX;
  
  return Generic_new(TYPE_FLOAT, p_res, 0);
}

/* control */
// (loop n f)
// applys f, n times, passing the current index to f
// returns whatever f returns if not void
// will go to completion and return void if void is all f returns
Generic *StdLib_loop(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_INT};
  enum Type allowedTypes2[] = {TYPE_NATIVEFUNCTION, TYPE_FUNCTION};

  validateType(allowedTypes1, 1, args[0]->type, 1, lineNumber, "loop");
  validateType(allowedTypes2, 2, args[1]->type, 2, lineNumber, "loop");

  validateMin(args[0]->p_val, 0, 1, lineNumber, "loop");
  
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

// (until stop f initial)
// runs until f returns stop
// passes the last returned value to f, as well as the current index
// intial acts like the first "state", unless another state supplied
Generic *StdLib_until(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 3, length, lineNumber);

  // verify 2nd arg is a function
  enum Type allowedTypes[] = {TYPE_NATIVEFUNCTION, TYPE_FUNCTION};
  validateType(allowedTypes, 2, args[1]->type, 2, lineNumber, "until");

  // set up state
  Generic *state;

  if (length == 3) {
    state = Generic_copy(args[2]);
  } else {
    state = Generic_new(TYPE_VOID, NULL, 0);
  }

  // flags and index
  int i = 0;

  // loop
  while (true) {

    // get index
    int *p_i = (int *) malloc(sizeof(int));
    *p_i = i;
    Generic *newArgs[2] = {Generic_copy(state), Generic_new(TYPE_INT, p_i, 0)};

    // callback
    Generic *res = applyFunc(args[1], p_scope, newArgs, 2, lineNumber);

    // handle state
    int comp = Generic_is(res, args[0]);
    if (comp) {
      Generic_free(res);
      return state;
    } else {
      Generic_free(state);
      state = res;
    }

    i++;
  }
}

// (if c f g*)
// applys f if c == 1
// applys g or nothing if c == 0
Generic *StdLib_if(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 3, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_INT};
  enum Type allowedTypes2[] = {TYPE_NATIVEFUNCTION, TYPE_FUNCTION};
  enum Type allowedTypes3[] = {TYPE_NATIVEFUNCTION, TYPE_FUNCTION};

  validateType(allowedTypes1, 1, args[0]->type, 1, lineNumber, "if");
  validateType(allowedTypes2, 2, args[1]->type, 2, lineNumber, "if");
  if (length == 3) validateType(allowedTypes3, 2, args[2]->type, 3, lineNumber, "if");
  
  validateBinary(args[0]->p_val, 1, lineNumber, "if");

  if (*((int *) args[0]->p_val) == 1) return applyFunc(args[1], p_scope, NULL, 0, lineNumber);
  else if (length == 3 && *((int *) args[0]->p_val) == 0) return applyFunc(args[2], p_scope, NULL, 0, lineNumber);

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (wait t)
// waits t seconds
Generic *StdLib_wait(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);
  enum Type allowedTypes[] = {TYPE_INT, TYPE_FLOAT};
  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "wait");
  int initialTime = clock();
  while (
    (((double) clock()) - ((double) initialTime)) / ((double) CLOCKS_PER_SEC)
    < (args[0]->type == TYPE_INT ? *((int *) args[0]->p_val) : *((double *) args[0]->p_val))
  ) {}
  
  return Generic_new(TYPE_VOID, NULL, 0);
}

/* types */
// (int x)
// returns x as an integer, if string, float, or int passed
Generic *StdLib_integer(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  enum Type allowedTypes[] = {TYPE_STRING, TYPE_INT, TYPE_FLOAT};
  validateType(allowedTypes, 3, args[0]->type, 1, lineNumber, "integer");

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

// (string x)
// returns x as a string, if string, float, or int passed
Generic *StdLib_string(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  enum Type allowedTypes[] = {TYPE_STRING, TYPE_INT, TYPE_FLOAT};
  validateType(allowedTypes, 3, args[0]->type, 1, lineNumber, "string");
  
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

// (float x)
// returns x as an float, if string, float, or int passed
Generic *StdLib_float(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  enum Type allowedTypes[] = {TYPE_STRING, TYPE_INT, TYPE_FLOAT};
  validateType(allowedTypes, 3, args[0]->type, 1, lineNumber, "float");

  double *p_res = malloc(sizeof(double));

  if (args[0]->type == TYPE_STRING) {
    char *str = *((char **) args[0]->p_val);
    *p_res = atof(str);
  } else if (args[0]->type == TYPE_FLOAT) {
    double f = *((double *) args[0]->p_val);
    *p_res = f;
  } else {
    int i = *((int *) args[0]->p_val);
    *p_res = (double) i;
  }

  return Generic_new(TYPE_FLOAT, p_res, 0);
}

// (type arg)
// returns a string with the type of arg
Generic *StdLib_type(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);
  
  // get type
  char *type = getTypeString(args[0]->type);
  
  // malloc memory for result and write
  char *res = malloc(sizeof(char) * (strlen(type) + 1));
  strcpy(res, type);
  res[strlen(type)] = '\0';

  // pointer to string
  char **p_res = (char **) malloc(sizeof(char *));
  *p_res = res;

  // return new generic
  return Generic_new(TYPE_STRING, p_res, 0);
}

/* list and string */
// (list args...)
// returns a new list with args as values
Generic *StdLib_list(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  return Generic_new(TYPE_LIST, List_new(args, length), 0);
}

// (length list)
// returns list length
Generic *StdLib_length(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);
  
  enum Type allowedTypes[] = {TYPE_LIST, TYPE_STRING};
  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "length");

  // create int
  int *p_res = (int *) malloc(sizeof(int));

  // get length and return
  if (args[0]->type == TYPE_LIST) *p_res = List_length((List *) args[0]->p_val);
  else *p_res = strlen(*((char **) args[0]->p_val));
  return Generic_new(TYPE_INT, p_res, 0);
}

// (join arg1 arg2 arg3 ...)
// returns args joined together
Generic *StdLib_join(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateMinArgCount(2, length, lineNumber);

  // validate first type
  enum Type allowedTypes[] = {TYPE_LIST, TYPE_STRING};
  validateType(allowedTypes, 2, args[0]->type, 1, lineNumber, "join");

  // validate rest of types
  enum Type selectedType[] = {args[0]->type};
  for (int i = 1; i < length; i++) {
    validateType(selectedType, 1, args[i]->type, i + 1, lineNumber, "join");
  }
  
  if (args[0]->type == TYPE_STRING) {
    // string case
    // calculate size (starts at 1 for \0)
    int stringSize = 1;
    for (int i = 0; i < length; i++) stringSize += strlen(*((char **) args[i]->p_val));

    // malloc memory
    char *res = (char *) malloc(sizeof(char) * stringSize);

    // copy / concat
    strcpy(res, *((char **) args[0]->p_val));
    for (int i = 1; i < length; i++) strcat(res, *((char **) args[i]->p_val));

    // malloc pointer
    char **p_res = (char **) malloc(sizeof(char *));
    *p_res = res;

    return Generic_new(TYPE_STRING, p_res, 0);
  } else {
    // list case
    // create array of lists
    List *lists[length];
    
    // populate
    for (int i = 0; i < length; i++) lists[i] = ((List *) args[i]->p_val);

    // return new generic using List_join
    return Generic_new(TYPE_LIST, List_join(lists, length), 0);
  }
}

// (get list index1) or (get list index1 index2)
// returns item from list/string, or sublist/substring
Generic *StdLib_get(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 3, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_LIST, TYPE_STRING};
  validateType(allowedTypes1, 2, args[0]->type, 1, lineNumber, "get");

  enum Type allowedTypes2[] = {TYPE_INT};
  validateType(allowedTypes2, 1, args[1]->type, 2, lineNumber, "get");
  if (length == 3) validateType(allowedTypes2, 1, args[2]->type, 3, lineNumber, "get");

  if (args[0]->type == TYPE_LIST) {
    int inputLength = List_length((List *) args[0]->p_val);
    validateRange(args[1]->p_val, 0, inputLength - 1, 2, lineNumber, "get");

    // single item from list
    if (length == 2) return List_get((List *) (args[0]->p_val), *((int *) args[1]->p_val));

    // multiple items from list
    else if (length == 3) {
      validateRange(args[2]->p_val, *((int *) args[1]->p_val) + 1, inputLength, 3, lineNumber, "get");

      return Generic_new(TYPE_LIST, List_sublist(
        (List *) (args[0]->p_val), 
        *((int *) args[1]->p_val), 
        *((int *) args[2]->p_val)
      ), 0);
    }

  } else if (args[0]->type == TYPE_STRING) {
    int inputLength = strlen(*((char **) args[0]->p_val));
    validateRange(args[1]->p_val, 0, inputLength - 1, 2, lineNumber, "get");

    if (length == 2) {
      // single item from string
      char *res = malloc(sizeof(char) * 2);
      res[0] = (*((char **) args[0]->p_val))[*((int *) args[1]->p_val)];
      res[1] = '\0';

      char **p_res = (char **) malloc(sizeof(char *));
      *p_res = res;

      return Generic_new(TYPE_STRING, p_res, 0);
    } else if (length == 3) {
      // mutliple items from string
      validateRange(args[2]->p_val, *((int *) args[1]->p_val) + 1, inputLength, 3, lineNumber, "get");

      // create substring
      int start = *((int *) args[1]->p_val);
      int end = *((int *) args[2]->p_val);

      char *res = malloc(sizeof(char) * (end - start + 1));
      strncpy(
        res, 
        &(*((char **) args[0]->p_val))[start], 
        end - start
      );
      
      res[end - start] = 0;

      // create pointer
      char **p_res = (char **) malloc(sizeof(char *));
      *p_res = res;

      // return
      return Generic_new(TYPE_STRING, p_res, 0);
    }
  }

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (insert list item index) or (insert list item)
// returns list or string, modified with item at index, if no index supplied, end is assumed
Generic *StdLib_insert(Scope *p_scope, Generic *args[], int length, int lineNumber) {

  // input validation
  validateArgCount(2, 3, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_LIST, TYPE_STRING};
  validateType(allowedTypes1, 2, args[0]->type, 1, lineNumber, "insert");

  enum Type allowedTypes2[] = {TYPE_STRING};
  if (args[0]->type == TYPE_STRING) 
    validateType(allowedTypes2, 1, args[1]->type, 2, lineNumber, "insert");

  if (length == 3) {
    enum Type allowedTypes3[] = {TYPE_INT};
    validateType(allowedTypes3, 1, args[2]->type, 3, lineNumber, "insert");

    int inputLength = args[0]->type == TYPE_LIST 
      ? List_length((List *) args[0]->p_val)
      : strlen(*((char **) args[0]->p_val));
    validateRange(args[2]->p_val, 0, inputLength, 3, lineNumber, "insert");
  }

  if (args[0]->type == TYPE_LIST) {
    
    // list case
    if (length == 2) {
      return Generic_new(TYPE_LIST, List_insert(
        (List *) (args[0]->p_val), args[1], List_length((List *) (args[0]->p_val))
      ), 0);
    } else if (length == 3) {
      return Generic_new(TYPE_LIST, List_insert(
        (List *) (args[0]->p_val), args[1], *((int *) args[2]->p_val)
      ), 0); 
    }
  } else if (args[0]->type == TYPE_STRING) {

    // string case
    if (length == 2) {

      // when an index is not supplied, put simply acts like join
      return StdLib_join(p_scope, args, length, lineNumber);
    } else if (length == 3) {
      int stringSize = strlen(*((char **) args[0]->p_val)) + strlen(*((char **) args[1]->p_val)) + 1;

      // malloc memory
      char *res = (char *) malloc(sizeof(char) * stringSize);


      // copy / concat
      strncpy(res, *((char **) args[0]->p_val), *((int *) args[2]->p_val));
      res[*((int *) args[2]->p_val)] = '\0';

      strcat(res, *((char **) args[1]->p_val));
      strcat(res, &((*((char **) args[0]->p_val))[*((int *) args[2]->p_val)]));

      // pointer
      char **p_res = (char **) malloc(sizeof(char *));
      *p_res = res;

      // return
      return Generic_new(TYPE_STRING, p_res, 0);
    }
  }

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (set list item index)
// sets index to item
Generic *StdLib_set(Scope *p_scope, Generic *args[], int length, int lineNumber) {

  // input validation
  validateArgCount(3, 3, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_LIST, TYPE_STRING};
  validateType(allowedTypes1, 2, args[0]->type, 1, lineNumber, "set");

  enum Type allowedTypes2[] = {TYPE_STRING};
  if (args[0]->type == TYPE_STRING) 
    validateType(allowedTypes2, 1, args[1]->type, 2, lineNumber, "set");

  enum Type allowedTypes3[] = {TYPE_INT};
  validateType(allowedTypes3, 1, args[2]->type, 3, lineNumber, "set");

  int inputLength = args[0]->type == TYPE_LIST 
    ? List_length((List *) args[0]->p_val)
    : strlen(*((char **) args[0]->p_val));
  validateRange(args[2]->p_val, 0, inputLength - 1, 3, lineNumber, "set");

  if (args[0]->type == TYPE_LIST) {
    // list case
    return Generic_new(TYPE_LIST, List_set(
      (List *) (args[0]->p_val), args[1], *((int *) args[2]->p_val)
    ), 0); 

  } else if (args[0]->type == TYPE_STRING) {
    char *target = *((char **) args[0]->p_val);
    char *item = *((char **) args[1]->p_val);
    int index = *((int *) args[2]->p_val);

    // string case
    // length of result
    int stringSize = index + (strlen(item) > strlen(target) - index ? strlen(item) : strlen(target) - index) + 1;

    // malloc memory
    char *res = (char *) malloc(sizeof(char) * stringSize);

    // copy / concat
    strncpy(res, target, index);
    res[index] = '\0';

    strcat(res, item);

    strncat(res, &(target[index]), stringSize - strlen(res) - 1);

    // pointer
    char **p_res = (char **) malloc(sizeof(char *));
    *p_res = res;

    // return
    return Generic_new(TYPE_STRING, p_res, 0);
  }

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (delete list index)
// deletes item from list and returns
Generic *StdLib_delete(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 3, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_LIST, TYPE_STRING};
  validateType(allowedTypes1, 2, args[0]->type, 1, lineNumber, "delete");

  enum Type allowedTypes2[] = {TYPE_INT};
  validateType(allowedTypes2, 1, args[1]->type, 2, lineNumber, "delete");
  if (length == 3) validateType(allowedTypes2, 1, args[2]->type, 3, lineNumber, "delete");

  if (args[0]->type == TYPE_LIST) {
    int inputLength = List_length((List *) args[0]->p_val);
    validateRange(args[1]->p_val, 0, inputLength - 1, 2, lineNumber, "delete");

    // list case
    if (length == 2) {
      return Generic_new(TYPE_LIST, List_delete((List *) (args[0]->p_val), *((int *) args[1]->p_val)), 0);
    } else if (length == 3) {
      validateRange(args[2]->p_val, *((int *) args[1]->p_val) + 1, inputLength, 3, lineNumber, "delete");

      // case where we must delete multiple items
      return Generic_new(TYPE_LIST, List_deleteMultiple(
        (List *) (args[0]->p_val), *((int *) args[1]->p_val), *((int *) args[2]->p_val)
      ), 0);
    }

  } else {
    // string case
    int inputLength = strlen(*((char **) args[0]->p_val));
    validateRange(args[1]->p_val, 0, inputLength - 1, 2, lineNumber, "delete");

    char *target = *((char **) args[0]->p_val);
    int index1 = *((int *) args[1]->p_val);

    if (length == 2) {

      // length of result (no +1, as we are removing a charechter)
      int stringSize = strlen(target);

      // malloc memory
      char *res = (char *) malloc(sizeof(char) * stringSize);

      // copy / concat
      strncpy(res, target, index1);
      res[index1] = '\0';
      strncat(res, &(target[index1 + 1]), stringSize - strlen(res) - 1);

      // pointer
      char **p_res = (char **) malloc(sizeof(char *));
      *p_res = res;

      // return
      return Generic_new(TYPE_STRING, p_res, 0);
    } else if (length == 3) {
      // mutliple items from string
      validateRange(args[2]->p_val, *((int *) args[1]->p_val) + 1, inputLength, 3, lineNumber, "delete");

      int index2 = *((int *) args[2]->p_val);

      // length of result
      int stringSize = strlen(target) + 1 - (index2 - index1);

      // malloc memory
      char *res = (char *) malloc(sizeof(char) * stringSize);

      // copy / concat
      strncpy(res, target, index1);
      res[index1] = '\0';
      strncat(res, &(target[index2]), stringSize - strlen(res) - 1);

      // pointer
      char **p_res = (char **) malloc(sizeof(char *));
      *p_res = res;

      // return
      return Generic_new(TYPE_STRING, p_res, 0); 
    }
  }

  return Generic_new(TYPE_VOID, NULL, 0);
}

// (map list fn)
// applys fn to every item in list, returns list with results
Generic *StdLib_map(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_LIST};
  validateType(allowedTypes1, 1, args[0]->type, 1, lineNumber, "map");

  enum Type allowedTypes2[] = {TYPE_FUNCTION, TYPE_NATIVEFUNCTION};
  validateType(allowedTypes2, 2, args[1]->type, 2, lineNumber, "map");

  // create copy of list
  Generic *res = Generic_copy(args[0]);
  ListNode *p_curr = ((List *) (res->p_val))->p_head;

  int i = 0;
  while (p_curr != NULL) {
    
    // create pointer for index, to create generic for args
    int *p_i = (int *) malloc(sizeof(int));
    *p_i = i;

    // apply function
    Generic *newArgs[] = {p_curr->p_val, Generic_new(TYPE_INT, p_i, 0)};
    p_curr->p_val = applyFunc(args[1], p_scope, newArgs, 2, lineNumber);

    // increment
    p_curr = p_curr->p_next;
    i++;
  }

  return res;
}

// (reduce list fn acc)
// applys fn to every item in list, returns single reduced item
// acc is initial accumulator (assumed to be void if not supplied)
Generic *StdLib_reduce(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 3, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_LIST};
  validateType(allowedTypes1, 1, args[0]->type, 1, lineNumber, "reduce");

  enum Type allowedTypes2[] = {TYPE_FUNCTION, TYPE_NATIVEFUNCTION};
  validateType(allowedTypes2, 2, args[1]->type, 2, lineNumber, "reduce");

  ListNode *p_curr = ((List *) (args[0]->p_val))->p_head;
  if (p_curr == NULL) {
    return Generic_new(TYPE_VOID, NULL, 0);
  }
  
  // create accumulator
  Generic *p_acc;
  if (length == 3) {
    p_acc = Generic_copy(args[2]);
  } else {
    p_acc = Generic_new(TYPE_VOID, NULL, 0);
  }
 
  // loop on every item from 2nd item on list
  int i = 0;
  while (p_curr != NULL) {
    
    // create pointer for index, to create generic for args
    int *p_i = (int *) malloc(sizeof(int));
    *p_i = i;
    
    // apply function
    p_curr->p_val->refCount++;
    Generic *newArgs[] = {p_acc, Generic_copy(p_curr->p_val), Generic_new(TYPE_INT, p_i, 0)};
    p_acc = applyFunc(args[1], p_scope, newArgs, 3, lineNumber);
    
    // increment
    p_curr = p_curr->p_next;
    i++;
  }

  return p_acc;
}

// (range n)
// returns list with range from 0 -> n - 1
Generic *StdLib_range(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(1, 1, length, lineNumber);

  enum Type allowedTypes[] = {TYPE_INT};
  validateType(allowedTypes, 1, args[0]->type, 1, lineNumber, "range");
  validateMin(args[0]->p_val, 0, 1, lineNumber, "range");

  // make list of arguments to pass to List_new
  int count = *((int *) args[0]->p_val);
  Generic *argList[count];
  
  for (int i = 0; i < count; i++) {
    int *p_i = (int *) malloc(sizeof(int));
    *p_i = i;
    argList[i] = Generic_new(TYPE_INT, p_i, 0);
  }

  Generic *res = Generic_new(TYPE_LIST, List_new(argList, count), 0);

  // free
  for (int i = 0; i < count; i++) {
    Generic_free(argList[i]);
  }

  return res;
}

// (find x item)
// returns index of item
Generic *StdLib_find(Scope *p_scope, Generic *args[], int length, int lineNumber) {
  validateArgCount(2, 2, length, lineNumber);

  enum Type allowedTypes1[] = {TYPE_STRING, TYPE_LIST};
  validateType(allowedTypes1, 2, args[0]->type, 1, lineNumber, "find");

  if (args[0]->type == TYPE_STRING) {

    // string case
    enum Type allowedTypes2[] = {TYPE_STRING};
    validateType(allowedTypes2, 1, args[1]->type, 2, lineNumber, "find");

    // find pointer to substring
    char *p_sub = strstr(*((char **) args[0]->p_val), *((char **) args[1]->p_val));

    // return void if not found
    if (p_sub == NULL) return Generic_new(TYPE_VOID, NULL, 0);

    // get index and return
    int *p_index = malloc(sizeof(int)); 
    *p_index = p_sub - *((char **) args[0]->p_val);

    return Generic_new(TYPE_INT, p_index, 0);
  } else {
    // list case
    // for each item
    ListNode *p_curr = ((List *) args[0]->p_val)->p_head;
    int i = 0;

    while (p_curr != NULL) {
      if (Generic_is(p_curr->p_val, args[1])) {
        // if found, return index
        int *p_index = (int *) malloc(sizeof(int));
        *p_index = i;

        return Generic_new(TYPE_INT, p_index, 0);
      }

      p_curr = p_curr->p_next;
      i++;
    }

    // if not found return void
    return Generic_new(TYPE_VOID, NULL, 0);
  }
}

// creates a new global scope
// skipArgs, the number of args to skip from the real argv
Scope *newGlobal(int argc, char *argv[], int skipArgs) {

  // initialize random number generator for (random)
  srand(time(NULL) + clock());

  // create global scope
  Scope *p_global = Scope_new(NULL);
  Generic *args[argc - skipArgs];

  // for each arg
  for (int i = skipArgs; i < argc; i++) {
    char **p_val = (char **) malloc(sizeof(char *));

    // create string
    char *val = (char *) malloc(sizeof(char) * (strlen(argv[i]) + 1));
    strcpy(val, argv[i]);
    *p_val = val;

    // add to args
    args[i - skipArgs] = Generic_new(TYPE_STRING, p_val, 0);
  }
  
  // add arguments and arguments count
  Scope_set(p_global, "arguments", Generic_new(TYPE_LIST, List_new(args, argc - skipArgs), 0));

  // free arguments memory (as List_new does a copy)
  for (int i = skipArgs; i < argc; i++) {
    Generic_free(args[i - skipArgs]);
    args[i - skipArgs] = NULL;
  }

  // add void
  Scope_set(p_global, "void", Generic_new(TYPE_VOID, NULL, 0));

  // populate global scope with stdlib functions
  /* IO */
  Scope_set(p_global, "print", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_print, 0));
  Scope_set(p_global, "input", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_input, 0));
  Scope_set(p_global, "rows", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_rows, 0));
  Scope_set(p_global, "columns", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_columns, 0));
  Scope_set(p_global, "read_file", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_read_file, 0));
  Scope_set(p_global, "write_file", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_write_file, 0));
  Scope_set(p_global, "event", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_event, 0));
  Scope_set(p_global, "use", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_use, 0));

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
  Scope_set(p_global, "power", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_power, 0));
  Scope_set(p_global, "random", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_random, 0));
  
  /* control */
  Scope_set(p_global, "loop", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_loop, 0));
  Scope_set(p_global, "until", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_until, 0));
  Scope_set(p_global, "if", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_if, 0));
  Scope_set(p_global, "wait", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_wait, 0));

  /* types */
  Scope_set(p_global, "integer", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_integer, 0));
  Scope_set(p_global, "string", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_string, 0));
  Scope_set(p_global, "float", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_float, 0));
  Scope_set(p_global, "type", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_type, 0));

  /* list and string */
  Scope_set(p_global, "list", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_list, 0));
  Scope_set(p_global, "length", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_length, 0));
  Scope_set(p_global, "join", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_join, 0));
  Scope_set(p_global, "get", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_get, 0));
  Scope_set(p_global, "insert", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_insert, 0));
  Scope_set(p_global, "set", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_set, 0));
  Scope_set(p_global, "delete", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_delete, 0));
  Scope_set(p_global, "map", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_map, 0));
  Scope_set(p_global, "reduce", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_reduce, 0));
  Scope_set(p_global, "range", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_range, 0));
  Scope_set(p_global, "find", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_find, 0));

  return p_global;
}