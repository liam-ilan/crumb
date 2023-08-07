#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "generic.h"
#include "ast.h"
#include "scope.h"
#include "eval.h"

// applys a func, given arguments
// used for callbacks from the standard library
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
    if (i < length - 1) printf(" ");
  }

  printf("\n");

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
  *p_res = 0;

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
  return applyFunc(args[0], p_scope, newArgs, length - 1, lineNumber);
}

// (loop n f)
// applys f, n times, passing the current index to f
// returns whatever f returns if not void
// will go to completion and return void if void is all f returns
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

  // loop
  for (int i = 0; i < *((int *) args[0].p_val); i++) {

    // get arg to pass to cb
    int *p_arg = (int *) malloc(sizeof(int));
    *p_arg = i;
    Generic newArgs[1] = {Generic_new(TYPE_INT, p_arg)};
    
    // call cb
    Generic res = applyFunc(args[1], p_scope, newArgs, 1, lineNumber);

    // if void type returned, free
    if (res.type != TYPE_VOID) return res;
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

  if (*((int *) args[0].p_val) == 1) return applyFunc(args[1], p_scope, NULL, 0, lineNumber);
  else if (length == 3 && *((int *) args[0].p_val) == 0) return applyFunc(args[2], p_scope, NULL, 0, lineNumber);
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

// (! a)
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

// (+ a b)
// returns a + b
Generic StdLib_add(Scope *p_scope, Generic args[], int length, int lineNumber) {
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

  // error handling for types
  if (args[0].type != TYPE_INT && args[0].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: + function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  } else if (args[1].type != TYPE_INT && args[1].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: + function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1].type)
    );
    exit(0);
  }
  
  if (args[0].type == TYPE_INT && args[1].type == TYPE_INT) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0].p_val) + *((int *) args[1].p_val);
    return Generic_new(TYPE_INT, p_res);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types

    *p_res = (args[0].type == TYPE_FLOAT ? *((double *) args[0].p_val) : *((int *) args[0].p_val))
      + (args[1].type == TYPE_FLOAT ? *((double *) args[1].p_val) : *((int *) args[1].p_val));

    return Generic_new(TYPE_FLOAT, p_res); 
  }
}

// (- a b)
// returns a - b
Generic StdLib_sub(Scope *p_scope, Generic args[], int length, int lineNumber) {
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

  // error handling for types
  if (args[0].type != TYPE_INT && args[0].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: - function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  } else if (args[1].type != TYPE_INT && args[1].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: - function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1].type)
    );
    exit(0);
  }
  
  if (args[0].type == TYPE_INT && args[1].type == TYPE_INT) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0].p_val) - *((int *) args[1].p_val);
    return Generic_new(TYPE_INT, p_res);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types

    *p_res = (args[0].type == TYPE_FLOAT ? *((double *) args[0].p_val) : *((int *) args[0].p_val))
      - (args[1].type == TYPE_FLOAT ? *((double *) args[1].p_val) : *((int *) args[1].p_val));

    return Generic_new(TYPE_FLOAT, p_res); 
  }
}

// (* a b)
// returns a * b
Generic StdLib_mult(Scope *p_scope, Generic args[], int length, int lineNumber) {
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

  // error handling for types
  if (args[0].type != TYPE_INT && args[0].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: * function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  } else if (args[1].type != TYPE_INT && args[1].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: * function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1].type)
    );
    exit(0);
  }
  
  if (args[0].type == TYPE_INT && args[1].type == TYPE_INT) {
    // case where we can return int
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0].p_val) * *((int *) args[1].p_val);
    return Generic_new(TYPE_INT, p_res);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types

    *p_res = (args[0].type == TYPE_FLOAT ? *((double *) args[0].p_val) : *((int *) args[0].p_val))
      * (args[1].type == TYPE_FLOAT ? *((double *) args[1].p_val) : *((int *) args[1].p_val));

    return Generic_new(TYPE_FLOAT, p_res); 
  }
}

// (/ a b)
// returns a / b
Generic StdLib_div(Scope *p_scope, Generic args[], int length, int lineNumber) {
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

  // error handling for types
  if (args[0].type != TYPE_INT && args[0].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: / function requires int or float type for it's 1st argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[0].type)
    );
    exit(0);
  } else if (args[1].type != TYPE_INT && args[1].type != TYPE_FLOAT) {
    // supplied too many args, throw error
    printf(
      "Runtime Error @ Line %i: / function requires int or float type for it's 2nd argument, %s type supplied instead.\n", 
      lineNumber, getTypeString(args[1].type)
    );
    exit(0);
  }
  
  if (
    args[0].type == TYPE_INT && args[1].type == TYPE_INT 
    && *((int *) args[0].p_val) % *((int *) args[1].p_val) == 0
  ) {
    // case where we can return int (a is divisible by b and a an b are ints)
    int *p_res = (int *) malloc(sizeof(int));
    *p_res = *((int *) args[0].p_val) / *((int *) args[1].p_val);
    return Generic_new(TYPE_INT, p_res);

  } else {
    // case where we must return float
    double *p_res = (double *) malloc(sizeof(double));

    // convert args to approriate types
    *p_res = (args[0].type == TYPE_FLOAT ? *((double *) args[0].p_val) : *((int *) args[0].p_val))
      / (args[1].type == TYPE_FLOAT ? *((double *) args[1].p_val) : *((int *) args[1].p_val));

    return Generic_new(TYPE_FLOAT, p_res); 
  }
}

// creates a new global scope
Scope *newGlobal() {
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
  Scope_set(p_global, "+", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_add));
  Scope_set(p_global, "-", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_sub));
  Scope_set(p_global, "*", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_mult));
  Scope_set(p_global, "/", Generic_new(TYPE_NATIVEFUNCTION, &StdLib_div));

  return p_global;
}