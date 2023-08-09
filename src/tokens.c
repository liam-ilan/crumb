#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"

// get token type as string, given enum
char* getTokenTypeString(enum TokenType type) {
  switch (type) {
    case TOK_ASSIGNMENT: return "assignment";
    case TOK_APPLYOPEN: return "applyopen";
    case TOK_APPLYCLOSE: return "applyclose";
    case TOK_FUNCOPEN: return "funcopen";
    case TOK_FUNCCLOSE: return "funcclose";
    case TOK_COMMA: return "comma";
    case TOK_ARROW: return "arrow";
    case TOK_RETURN: return "return";
    case TOK_IDENTIFIER: return "identifier";
    case TOK_INT: return "int";
    case TOK_FLOAT: return "float";
    case TOK_STRING: return "string";
    case TOK_START: return "start";
    case TOK_END: return "end";
    default: return "unknown";
  }
}

// print token list
void Token_print(Token *p_head, int length) {
  Token *p_curr = p_head;
  int i = 0;
  while (p_curr != NULL && i < length) {
    if (p_curr->val == NULL) printf("%i| %s\n", p_curr->lineNumber, getTokenTypeString(p_curr->type));
    else printf("%i| %s: %s\n", p_curr->lineNumber, getTokenTypeString(p_curr->type), p_curr->val);
    
    i++;
    p_curr = p_curr->p_next;
  }
}

// add token to list
void Token_push(Token *p_head, char *val, enum TokenType type, int lineNumber) {
  // loop to end of list
  Token *p_curr = p_head;
  while (p_curr->p_next != NULL) {
    p_curr = p_curr->p_next;
  }

  // allocate memory for new token
  Token *p_newToken = (Token *)(malloc(sizeof(Token)));

  // add new node to end of list
  p_curr->p_next = p_newToken;

  // write data
  p_curr->p_next->val = val;
  p_curr->p_next->type = type;
  p_curr->p_next->p_next = NULL;
  p_curr->p_next->lineNumber = lineNumber;
}

// frees all tokens, and their values
void Token_free(Token *p_head) {
  Token *p_curr = p_head;
  Token *p_tmp = p_curr;

  while (p_curr != NULL) {
    p_tmp = p_curr;
    p_curr = p_curr->p_next;

    free(p_tmp->val);
    free(p_tmp);
  }
}