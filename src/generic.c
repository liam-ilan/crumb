#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generic.h"
#include "ast.h"

// print generic nicely
void Generic_print(Generic *in) {
  if (in->type == TYPE_INT) {
    printf("%i", *((int *) in->p_val));
  } else if (in->type == TYPE_FLOAT) {
    printf("%f", *((double *) in->p_val));
  } else if (in->type == TYPE_STRING) {
    printf("%s", *((char **) in->p_val));
  } else if (in->type == TYPE_VOID) {
    printf("[Void]");
  } else if (in->type == TYPE_FUNCTION) {
    printf("[Function]");
  }  else if (in->type == TYPE_NATIVEFUNCTION) {
    printf("[Native Function]");
  }
}

// create a new generic and return
Generic* Generic_new(enum Type type, void *p_val, int refCount) {
  Generic *res = malloc(sizeof(Generic));
  res->type = type;
  res->p_val = p_val;
  res->refCount = refCount;
  return res;
}

// frees p_val of generic 
void Generic_free(Generic *target) {
  // if string, free contents as well
  if (target->type == TYPE_STRING) free(*((char **) target->p_val));

  // dont free functions and native functions, as they are either not allocated to heap (native), or belong to the ast (non native)
  if (target->type != TYPE_FUNCTION && target->type != TYPE_NATIVEFUNCTION) free(target->p_val);
  target->p_val = NULL;

  // free generic itself
  free(target);
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
    default: return "unknown";
  }
}

Generic *Generic_copy(Generic *target) {
  Generic *res = (Generic *) malloc(sizeof(Generic));
  res->type = target->type;
  res->refCount = 0;

  if (res->type == TYPE_STRING) {
    res->p_val = (char **) malloc(sizeof(char *));
    *((char **) res->p_val) = malloc(sizeof(char) * (strlen(*((char **) target->p_val)) + 1));
    strcpy(*((char **) res->p_val), *((char **) target->p_val));
  } else if (res->type == TYPE_FUNCTION || res->type == TYPE_NATIVEFUNCTION) {
    res->p_val = target->p_val;
  } else if (res->type == TYPE_VOID) {
    res->p_val = NULL;
  } else if (res->type == TYPE_INT) {
    res->p_val = (int *) malloc(sizeof(int));
    *((int *) res->p_val) = *((int *) target->p_val);
  } else if (res->type == TYPE_FLOAT) {
    res->p_val = (double *) malloc(sizeof(double));
    *((double *) res->p_val) = *((double *) target->p_val);
  }

  return res;
}