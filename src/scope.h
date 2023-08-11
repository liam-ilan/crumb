#ifndef SCOPE_H
#define SCOPE_H
#include "generic.h"

// a key value pair held in Scope (linked list)
typedef struct ScopeItem {
  char* key;
  Generic *p_val;
  struct ScopeItem *p_next;
} ScopeItem;

// scope (assigned to every statement)
// every scope knows its parent, so that if a var is not in local scope, parent scope can be accesed
// a scope contains a linked map of var names and values
typedef struct Scope {
  struct Scope *p_parent;
  ScopeItem *p_head;
} Scope;

// prototypes
ScopeItem *ScopeItem_new(char*, Generic *);
Scope *Scope_new(Scope *);
void Scope_print(Scope *);
void Scope_set(Scope *, char *, Generic *);
Generic *Scope_get(Scope *, char *, int);
void Scope_free(Scope *);
Scope* Scope_copy(Scope *);


#endif