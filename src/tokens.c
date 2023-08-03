#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"

// get token type as string, given enum
char* getTokenTypeString(enum TokenType type) {
  switch (type) {
    case ASSIGNMENT: return "assignment";
    case APPLYOPEN: return "applyopen";
    case APPLYCLOSE: return "applyclose";
    case FUNCOPEN: return "funcopen";
    case FUNCCLOSE: return "funcclose";
    case COMMA: return "comma";
    case ARROW: return "arrow";
    case RETURN: return "return";
    case IDENTIFIER: return "identifier";
    case INT: return "int";
    case FLOAT: return "float";
    case STRING: return "string";
    case START: return "start";
    case END: return "end";
  }
}

// print token list
void Token_print(Token *p_head, int length) {
  Token *p_curr = p_head;
  int i = 0;
  while (p_curr != NULL && i < length) {
    if (p_curr->val == NULL) printf("%i | %s\n", p_curr->lineNumber, getTokenTypeString(p_curr->type));
    else printf("%i | %s: %s\n", p_curr->lineNumber, getTokenTypeString(p_curr->type), p_curr->val);
    
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