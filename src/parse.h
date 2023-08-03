#ifndef PARSE_H
#define PARSE_H
#include "tokens.h"
#include "ast.h"

// prototypes
void skipClosure(int *, Token **, enum TokenType, enum TokenType, int);

AstNode *parseValue(Token *, int);
AstNode *parseAssignment(Token *, int);
AstNode *parseStatement(Token *, int);
AstNode *parseApplication(Token *, int);
AstNode *parseReturn(Token *, int);
AstNode *parseFunction(Token *, int);
AstNode *parseProgram(Token *, int);

#endif