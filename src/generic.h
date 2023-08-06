#ifndef GENERIC_H
#define GENERIC_H

// types for generic
enum Type {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_VOID,
  TYPE_FUNCTION,
  TYPE_NATIVEFUNCTION
};

// generic struct
// p_val: a void pointer to the value
// type: the type of *p_val
// note - We tend to pass around generic by value, not by pointer, unlike tokens and ast nodes.
typedef struct Generic {
  enum Type type;
  void *p_val;
} Generic;

// prototypes
char* getTypeString(enum Type);
void Generic_print(Generic);
Generic Generic_new(enum Type, void *);
void Generic_free(Generic);
#endif