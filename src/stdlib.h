#ifndef STDLIB_H
#define STDLIB_H
#include "generic.h"
#include "scope.h"

// prototypes
Scope *newGlobal(int, char **, int);
Generic *applyFunc(Generic *, Scope *, Generic *[], int, int);

#endif