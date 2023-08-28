#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lex.h"
#include "tokens.h"
#include "string.h"

// handles errors while scanning chars in a string
void handleStringError(char c, int lineNumber) {
  if (c == '\n') {
    printf("Syntax Error @ Line %i: Unexpected new line before string closed.\n", lineNumber);
    exit(0);
  }

  if (c == '\0') {
    printf("Syntax Error @ Line %i: Unexpected end of file before string closed.\n", lineNumber);
    exit(0);
  }
}

// lex code with length fileLength into tokens
int lex(Token *p_headToken, char *code, int fileLength) {

  // token count
  int tokenCount = 1;

  // line number
  int lineNumber = 1;

  // for each char (including terminator, helps us not need to push number tokens if they are last)
  int i = 0;
  while (i < fileLength) {
    char c = code[i];

    if (c == '\n') lineNumber++;

    if (c == '/' && code[i + 1] == '/') {
      // comments
      while (code[i] != '\n' && i < fileLength) i++;
      lineNumber++;
    } else if (c == '(') {
      Token_push(p_headToken, NULL, TOK_APPLYOPEN, lineNumber);
      tokenCount++;
    } else if (c == ')') {
      Token_push(p_headToken, NULL, TOK_APPLYCLOSE, lineNumber);
      tokenCount++;
    } else if (c == '=') {
      Token_push(p_headToken, NULL, TOK_ASSIGNMENT, lineNumber);
      tokenCount++;
    } else if (c == '{') {
      Token_push(p_headToken, NULL, TOK_FUNCOPEN, lineNumber);
      tokenCount++;
    } else if (c == '}') {
      Token_push(p_headToken, NULL, TOK_FUNCCLOSE, lineNumber);
      tokenCount++;
    } else if (c == '-' && code[i + 1] == '>') {
      Token_push(p_headToken, NULL, TOK_ARROW, lineNumber);
      i++;
      tokenCount++;
    } else if (c == '<' && code[i + 1] == '-') {
      Token_push(p_headToken, NULL, TOK_RETURN, lineNumber);
      i++;
      tokenCount++;
    } else if (c == '"') {
      
      // record first char in string
      int stringStart = i + 1;

      // go to first char after quotes
      i++;

      // count to last char in string (last quote)
      while (code[i] != '"') {

        // error handling
        handleStringError(code[i], lineNumber);
        
        // skip escape codes
        if (code[i] == '\\') {
          i++;
          handleStringError(code[i], lineNumber);
        }
        
        i++;
      }

      // get substring and add token
      char *val = malloc(i - stringStart + 1);
      strncpy(val, &code[stringStart], i - stringStart);
      val[i - stringStart] = '\0';

      Token_push(p_headToken, parseString(val), TOK_STRING, lineNumber);

      free(val);
      tokenCount++;

    } else if (isdigit(c) > 0 || (c == '-' && isdigit(code[i + 1]) > 0)) {
      
      // record first char in int
      int numStart = i;

      // float flag
      bool isFloat = false;

      // increment
      i++;

      // increment until char is not a valid number char
      while (isdigit(code[i]) > 0 || code[i] == '.') {
        
        // handle float flag and protect against multiple points
        if (code[i] == '.') {
          if (isFloat) {
            // case were we saw a point before
            printf("Syntax Error @ Line %i: Multiple decimal points in single number.\n", lineNumber);
            exit(0);
          } else isFloat = true;
        }

        i++;
      }

      // get substring and add token
      char *val = malloc(i - numStart + 1);
      strncpy(val, &code[numStart], i - numStart);
      val[i - numStart] = '\0';

      if (isFloat) Token_push(p_headToken, val, TOK_FLOAT, lineNumber);
      else Token_push(p_headToken, val, TOK_INT, lineNumber);
      tokenCount++;

      // make sure to go back to last char of number
      i--;

    } else if (
      strchr(" \n\r\t\f\v{}()\"=", c) == NULL 
      && !(c == '-' && code[i + 1] == '>')
      && !(c == '<' && code[i + 1] == '-')
      && !(c == '/' && code[i + 1] == '/')
    ) {
      // case of identifier
      int identifierStart = i;

      // while valid identifier char
      while (
        strchr(" \n\r\t\f\v{}()\"=", code[i]) == NULL 
        && !(code[i] == '-' && code[i + 1] == '>') 
        && !(code[i] == '<' && code[i + 1] == '-')
        && !(code[i] == '/' && code[i + 1] == '/')
      ) i++;

      // get substring and add token
      char *val = malloc(i - identifierStart + 1);
      strncpy(val, &code[identifierStart], i - identifierStart);
      val[i - identifierStart] = '\0';

      Token_push(p_headToken, val, TOK_IDENTIFIER, lineNumber);

      tokenCount++;
      
      // make sure to go back to last char of identifier
      i--;
    } else if (strchr(" \n\r\t\f\v", code[i]) == NULL) {
      // handle unexpected char
      printf("Syntax Error @ Line %i: Unexpected char \"%c\".\n", lineNumber, c);
      exit(0);
    }

    i++;
  }

  Token_push(p_headToken, NULL, TOK_END, lineNumber); 
  tokenCount++;

  return tokenCount;
}