#include <stdio.h>
#include "function.h"
#include "scope.h"
#include "ast.h"

Function *Function_new(AstNode *p_ast, Scope *p_scope) {

  Function *res = (Function *) malloc(sizeof(Function));
  res->p_ast = p_ast;
  res->p_scope = Scope_copy(p_scope);

  return res;
}

void Function_free(Function *p_target) {
  Scope_free(p_target->p_scope);
  free(p_target);
}

Function *Function_copy(Function *p_target) {
  Function *res = malloc(sizeof(Function));
  res->p_ast = p_target->p_ast;
  res->p_scope = Scope_copy(p_target->p_scope);

  return res;
}