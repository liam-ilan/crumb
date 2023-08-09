#ifndef TOKENS_H
#define TOKENS_H

enum TokenType {
  TOK_ASSIGNMENT, 
  TOK_APPLYOPEN, 
  TOK_APPLYCLOSE, 
  TOK_FUNCOPEN, 
  TOK_FUNCCLOSE, 
  TOK_COMMA, 
  TOK_ARROW, 
  TOK_RETURN, 
  TOK_IDENTIFIER, 
  TOK_INT, 
  TOK_FLOAT,
  TOK_STRING,
  TOK_END,
  TOK_START
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
void Token_free(Token *);

#endif