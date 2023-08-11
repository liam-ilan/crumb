#ifndef FUNCTION_H
#define FUNCTION_H
#include <stdlib.h>
#include "function.h"
#include "generic.h"
#include "ast.h"
#include "scope.h"

// function struct
// p_ast: a pointer to the head ast node of the function
// p_scope: a copy of the scope in which the function was defined
typedef struct Function {
  AstNode *p_ast;
  Scope *p_scope;
} Function;

// prototypes
Function *Function_new(AstNode *, Scope *);
void Function_free(Function *);
Function *Function_copy(Function *);

#endif