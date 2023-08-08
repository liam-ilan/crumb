#ifndef EVAL_H
#define EVAL_H
#include "generic.h"
#include "ast.h"
#include "scope.h"
// prototype
Generic *eval(AstNode *p_head, Scope *p_scope);

#endif