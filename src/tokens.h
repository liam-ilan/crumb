#ifndef TOKENS_H
#define TOKENS_H

enum TokenType {
  ASSIGNMENT, 
  APPLYOPEN, 
  APPLYCLOSE, 
  FUNCOPEN, 
  FUNCCLOSE, 
  COMMA, 
  ARROW, 
  RETURN, 
  IDENTIFIER, 
  INT, 
  FLOAT,
  STRING,
  END,
  START
};

// token
// linked list element containing pointer to next token, type of token, and value
typedef struct Token {
  char *val;
  enum TokenType type;
  struct Token *p_next;
  int lineNumber;
} Token;

// prototypes
char* getTokenTypeString(enum TokenType);
void Token_print(Token *, int);
void Token_push(Token *, char *, enum TokenType, int);

#endif