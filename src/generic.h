#ifndef GENERIC_H
#define GENERIC_H

// types for generic
enum Type {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_VOID,
  TYPE_FUNCTION,
  TYPE_NATIVEFUNCTION,
  TYPE_LIST
};

// generic struct
// p_val: a void pointer to the value
// type: the type of *p_val
typedef struct Generic {
  enum Type type;
  void *p_val;
  int refCount;
} Generic;

// prototypes
char* getTypeString(enum Type);
void Generic_print(Generic *);
Generic *Generic_new(enum Type, void *, int refCount);
void Generic_free(Generic *);
Generic *Generic_copy(Generic *);
int Generic_is(Generic *, Generic *);
#endif