#include <stdio.h>
#include <stdlib.h>
#include "generic.h"
#include "ast.h"

// print generic nicely
void Generic_print(Generic in) {
  if (in.type == TYPE_INT) {
    printf("%i", *((int *) in.p_val));
  } else if (in.type == TYPE_FLOAT) {
    printf("%f", *((double *) in.p_val));
  } else if (in.type == TYPE_STRING) {
    printf("%s", *((char **) in.p_val));
  } else if (in.type == TYPE_VOID) {
    printf("[Void]");
  } else if (in.type == TYPE_FUNCTION) {
    printf("[Function]");
  }  else if (in.type == TYPE_NATIVEFUNCTION) {
    printf("[Native Function]");
  }
}

// create a new generic and return
Generic Generic_new(enum Type type, void *p_val) {
  Generic res = {type, p_val};
  return res;
}

// free generic
// note: we are actually just freeing the val of generic
// protects against freeing functions (as they store ast nodes) and strings (as they r properties of ast nodes)
void Generic_free(Generic in) {
  if (in.type != TYPE_FUNCTION) free(in.p_val);
}


// returns type as a string given enum
char *getTypeString(enum Type type) {
  switch (type) {
    case TYPE_FLOAT: return "float";
    case TYPE_INT: return "int";
    case TYPE_FUNCTION: return "function";
    case TYPE_STRING: return "string";
    case TYPE_VOID: return "float";
    case TYPE_NATIVEFUNCTION: return "native function";
  }
}