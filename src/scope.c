#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scope.h"
#include "generic.h"

// creates a new scope item, allocates memory, returns pointer
ScopeItem *ScopeItem_new(char* key, Generic *p_val) {
  ScopeItem *res = (ScopeItem *) malloc(sizeof(ScopeItem));
  res->key = key;
  res->p_val = p_val;
  res->p_next = NULL;

  return res;
}

// creates a new empty scope, allocates memory, and returns a pointer
Scope *Scope_new(Scope *p_parent) {
  Scope *res = (Scope *) malloc(sizeof(Scope));
  res->p_parent = p_parent;
  res->p_head = NULL;

  return res;
}

// nicely prints scope, given pointer
void Scope_print(Scope *p_in) {
  if (p_in->p_parent == NULL) printf("Global Scope:\n");
  else printf("Local Scope:\n");

  // for each pair, print
  ScopeItem *p_curr = p_in->p_head;
  while (p_curr != NULL) {
    printf("%s = ", p_curr->key);
    Generic_print(p_curr->p_val);
    printf("\n");
    p_curr = p_curr->p_next;
  }
}

// gets a pointer to the scope
// sets a key in the scope to val
// if the key does not exist, creates a new scope item to house it
void Scope_set(Scope *p_target, char *key, Generic *p_val) {
  p_val->refCount++;

  char *keyCopy = (char *) malloc(sizeof(char) * (strlen(key) + 1));
  strcpy(keyCopy, key);

  // set p_p_curr to the ScopeItem with the correct key, or NULL if not found
  ScopeItem **p_p_curr = &(p_target->p_head);
  while (*(p_p_curr) != NULL && strcmp((*p_p_curr)->key, keyCopy) != 0) p_p_curr = &((*p_p_curr)->p_next);

  if (*p_p_curr == NULL) {
    // case where variable was previously undefined, create new item
    *p_p_curr = ScopeItem_new(keyCopy, p_val);
  } else {

    // case where variable was previosuly defined, simply overwrite value, and decrease ref count of old value (if 0, free)
    (*p_p_curr)->p_val->refCount--;
    if ((*p_p_curr)->p_val->refCount == 0) {
      Generic_free((*p_p_curr)->p_val);
    } 
    (*p_p_curr)->p_val = p_val;
  }
}

// returns the generic in the requested key of the target scope
// if the generic cannot be found, attempts to search parent recursively
Generic *Scope_get(Scope *p_target, char *key, int lineNumber) {

  // set p_curr to the item with correct key, or NULL
  ScopeItem *p_curr = p_target->p_head;
  while (p_curr != NULL && strcmp(p_curr->key, key) != 0) p_curr = p_curr->p_next;

  if (p_curr == NULL) {
    // key does not exist in current scope
    // if parent does not exist, we are in the global scope, and can throw an error, else elevate
    if (p_target->p_parent == NULL) {
      // Error handling
      printf(
        "Runtime Error @ Line %i: %s is not defined.\n", 
        lineNumber, key
      );
      exit(0);
    } else {
      return Scope_get(p_target->p_parent, key, lineNumber);
    }
  } else {
    // if key exists, return generic
    return p_curr->p_val;
  }
}

// free Scope and ScopeItems from memory
// returns the generic in the requested key of the target scope
// if the generic cannot be found, attempts to search parent recursively
void Scope_free(Scope *p_target) {
  // set p_curr to the item with correct key, or NULL
  ScopeItem *p_curr = p_target->p_head;

  while (p_curr != NULL) {
    ScopeItem *p_tmp = p_curr;
    p_curr = p_curr->p_next;

    p_tmp->p_val->refCount--;
    if (p_tmp->p_val->refCount == 0) Generic_free(p_tmp->p_val);
    
    free(p_tmp->key);
    free(p_tmp);
  }

  free(p_target);
}