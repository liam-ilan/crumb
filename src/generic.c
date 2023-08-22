#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generic.h"
#include "ast.h"
#include "list.h"

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
  } else if (in->type == TYPE_NATIVEFUNCTION) {
    printf("[Native Function]");
  } else if (in->type == TYPE_LIST) {
    List_print((List *) (in->p_val));
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

  if (target->type == TYPE_LIST) {
    List_free((List *) (target->p_val)); // use list's own free function
  } else if (target->type == TYPE_FUNCTION) {
    AstNode_free(target->p_val); // functions are in reality ast nodes, so free them with the appropriate function
  } else if (target->type != TYPE_NATIVEFUNCTION) {
    // dont free native functions, as their void pointers are not allocated to heap
    free(target->p_val);
  }
  
  target->p_val = NULL;

  // free generic itself
  free(target);
}

// returns type as a string given enum
char *getTypeString(enum Type type) {
  switch (type) {
    case TYPE_FLOAT: return "float";
    case TYPE_INT: return "integer";
    case TYPE_FUNCTION: return "function";
    case TYPE_STRING: return "string";
    case TYPE_VOID: return "void";
    case TYPE_NATIVEFUNCTION: return "native function";
    case TYPE_LIST: return "list";
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
  } else if (res->type == TYPE_FUNCTION) {
    res->p_val = AstNode_copy(target->p_val, 0);
  } else if (res->type == TYPE_NATIVEFUNCTION) {
    res->p_val = target->p_val;
  } else if (res->type == TYPE_VOID) {
    res->p_val = NULL;
  } else if (res->type == TYPE_INT) {
    res->p_val = (int *) malloc(sizeof(int));
    *((int *) res->p_val) = *((int *) target->p_val);
  } else if (res->type == TYPE_FLOAT) {
    res->p_val = (double *) malloc(sizeof(double));
    *((double *) res->p_val) = *((double *) target->p_val);
  } else if (res->type == TYPE_LIST) {
    res->p_val = List_copy((List *) target->p_val);
  }

  return res;
}

// returns 1 if a and b are the same, else returns 0
int Generic_is(Generic *a, Generic *b) {
  int res = 0;

  // case where we want numerical equality (is 1.0 1) -> 1
  if (
    (a->type == TYPE_INT || a->type == TYPE_FLOAT)
    && (b->type == TYPE_INT || b->type == TYPE_FLOAT)
  ) {
    return (
      (a->type == TYPE_FLOAT ? *((double *) a->p_val) : *((int *) a->p_val))
      == (b->type == TYPE_FLOAT ? *((double *) b->p_val) : *((int *) b->p_val))
    );
  }

  // else check types
  if (a->type == b->type) {

    // do type conversions and check data
    switch (a->type) {
      case TYPE_FLOAT:
        if (*((double *) a->p_val) == *((double *) b->p_val)) res = 1;
        break;
      case TYPE_INT:
        if (*((int *) a->p_val) == *((int *) b->p_val)) res = 1;
        break;
      case TYPE_STRING:
        if (strcmp(*((char **) a->p_val), *((char **) b->p_val)) == 0) res = 1;
        break;
      case TYPE_VOID:
        res = 1;
        break;
      case TYPE_NATIVEFUNCTION:
        if (a->p_val == b->p_val) res = 1;
        break;
      case TYPE_FUNCTION:
        if (a->p_val == b->p_val) res = 1;
        break;
      case TYPE_LIST:
        res = 1;

        // get list nodes
        ListNode *p_aCurr = ((List *) (a->p_val))->p_head;
        ListNode *p_bCurr = ((List *) (b->p_val))->p_head;

        // for each item
        while (p_aCurr != NULL && p_bCurr != NULL) {
          int comp = Generic_is(p_aCurr->p_val, p_bCurr->p_val);
          res = res * comp;

          p_aCurr = p_aCurr->p_next;
          p_bCurr = p_bCurr->p_next;
        }

        // if not the same length
        if (p_aCurr != NULL || p_bCurr != NULL) {
          res = 0;
        }
    }
  }

  return res;
}