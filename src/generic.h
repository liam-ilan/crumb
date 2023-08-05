#ifndef GENERIC_H
#define GENERIC_H

// types for generic
enum Type {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_VOID,
  TYPE_FUNCTION
};

// generic struct
// p_val: a void pointer to the value
// type: the type of *p_val
// note - We tend to pass around generic by value, not by pointer, unlike tokens and ast nodes.
typedef struct Generic {
  enum Type type;
  void *p_val;
} Generic;

// prototype
void Generic_print(Generic);
#endif