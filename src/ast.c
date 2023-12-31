#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// frees ast
void AstNode_free(AstNode *p_head) {
  // for each child, free memory
  AstNode *p_curr = p_head->p_headChild;
  AstNode *p_tmp = NULL;
  
  while (p_curr != NULL) {
    p_tmp = p_curr;
    p_curr = p_curr->p_next;
    AstNode_free(p_tmp);
  }

  // free self
  free(p_head->val);
  free(p_head);
}

// converts opcode to string for printing
char* getOpcodeString(enum Opcodes code) {
  switch (code) {
    case OP_INT: return "int";
    case OP_FLOAT: return "float";
    case OP_STRING: return "string";
    case OP_IDENTIFIER: return "identifier";
    case OP_ASSIGNMENT: return "assignment";
    case OP_RETURN: return "return";
    case OP_STATEMENT: return "statement";
    case OP_APPLICATION: return "application";
    case OP_FUNCTION: return "function";
    default: return "unknown";
  }
}

// print ast nicely
void AstNode_print(AstNode *p_head, int depth) {

  // print current node
  if (p_head->val == NULL) printf("%i| %s\n", p_head->lineNumber, getOpcodeString(p_head->opcode));
  else printf("%i| %s: %s\n", p_head->lineNumber, getOpcodeString(p_head->opcode), p_head->val);

  // for each child
  AstNode *p_curr = p_head->p_headChild;
  while (p_curr != NULL) {
    
    // create appropriate whitespace and dash
    for (int x = 0; x < depth + 1; x++) printf("   ");

    // print child
    AstNode_print(p_curr, depth + 1);

    p_curr = p_curr->p_next;
  }
}

// appened an item as child to p_head, given a pointer to p_lastChild
// if p_lastChild is null, there are no current children
void AstNode_appendChild(AstNode *p_head, AstNode **p_p_lastChild, AstNode *p_child) {
  if (*p_p_lastChild == NULL) {
    p_head->p_headChild = p_child;
    *p_p_lastChild = p_head->p_headChild;
  } else {
    (*p_p_lastChild)->p_next = p_child;
    *p_p_lastChild = (*p_p_lastChild)->p_next;
  }
}

// allocates memory for a new ast node and populates it
AstNode* AstNode_new(char* val, enum Opcodes opcode, int lineNumber) {
  AstNode *res = (AstNode *) malloc(sizeof(AstNode));
  res->opcode = opcode;
  res->p_headChild = NULL;
  res->p_next = NULL;
  res->val = NULL;

  if (val != NULL) {
    res->val = (char *) malloc(sizeof(char) * (strlen(val) + 1));
    strcpy(res->val, val);
  }


  res->lineNumber = lineNumber;
  return res;
}

// creats a copy of the astnode, recursively traversing through its children/siblings, and returns it
// depth is the current depth of the copy (start at 0), if 0, do not copy siblings
AstNode* AstNode_copy(AstNode *p_head, int depth) {
  if (p_head == NULL) return NULL;
  AstNode* p_res = AstNode_new(p_head->val, p_head->opcode, p_head->lineNumber);
  p_res->p_headChild = AstNode_copy(p_head->p_headChild, depth + 1);
  
  if (depth != 0) p_res->p_next = AstNode_copy(p_head->p_next, depth + 1);
  else p_res->p_next = NULL;

  return p_res;
}