#ifndef STDLIB_H
#define STDLIB_H
#include "generic.h"
#include "scope.h"

// prototypes
Scope *newGlobal();
Generic applyFunc(Generic, Scope *, Generic[], int, int);

Generic StdLib_print(Scope *, Generic[], int, int);
Generic StdLib_is(Scope *, Generic[], int, int);
Generic StdLib_apply(Scope *, Generic[], int, int);
Generic StdLib_loop(Scope *, Generic[], int, int);
Generic StdLib_if(Scope *, Generic[], int, int);
Generic StdLib_mod(Scope *, Generic[], int, int);
Generic StdLib_not(Scope *, Generic[], int, int);

#endif