#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include "ast.h"
#include "tokens.h"

// skips closure, such as function or application
// particulary useful for parsing statement
// p_p_curr is the pointer to the pointer to the token containing the open of the closure ("(" or "{")
// open and close are the respective token types designating the open and close types
// p_index is a pointer to the loop itterating over the tokens
// length is the length of the parent expression
void skipClosure(int *p_index, Token **p_p_curr, enum TokenType open, enum TokenType close, int length) {
  int depth = 1;

  // until depth == 0 (we are out of the application), loop
  while (depth != 0 && *p_index < length) {
    *p_p_curr = (*p_p_curr)->p_next;
    (*p_index)++;

    // throw bug if we reach the end of a file
    if ((*p_p_curr)->type == TOK_END) {
      printf(
        "Syntax Error @ Line %i: Unexpected %s token.\n", 
        (*p_p_curr)->lineNumber, getTokenTypeString((*p_p_curr)->type)
      );
      exit(0);
    }

    if ((*p_p_curr)->type == close) depth--;
    else if ((*p_p_curr)->type == open) depth++;
  }
}

// parse application
// ebnf: application = "(", {value}, ")";
AstNode *parseApplication(Token *p_head, int length) {
  if (p_head->type != TOK_APPLYOPEN) {
    // error handling for invalid first token
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );
    exit(0);
  }

  AstNode *res = AstNode_new(NULL, OP_APPLICATION, p_head->lineNumber);
  AstNode *p_lastChild = NULL;

  // go to first token of first item item in application
  Token *p_curr = p_head->p_next;
  int i = 1;

  while(p_curr->type != TOK_APPLYCLOSE && i < length) {

    if (p_curr->type == TOK_APPLYOPEN || p_curr->type == TOK_FUNCOPEN) {
      // case of value which is closure
      // record first token
      Token *p_start = p_curr;
      int startIndex = i;
    
      // skip closure
      skipClosure(&i, &p_curr, p_curr->type, p_curr->type + 1, length);

      // add child
      AstNode_appendChild(res, &p_lastChild, parseValue(p_start, i - startIndex + 1));
    } else {
      // case of value not in closure
      AstNode_appendChild(res, &p_lastChild, parseValue(p_curr, 1));
    }

    p_curr = p_curr->p_next;
    i++;
  }
  
  if (p_curr->type != TOK_APPLYCLOSE) {
    // error handling for invalid closing token
    printf(
      "Syntax Error @ Line %i: Application not closed.\n", 
      p_curr->lineNumber
    );
    exit(0);
  }

  return res;
}

// parse value
// ebnf: value = application | function | int | float | string | identifier;
AstNode *parseValue(Token *p_head, int length) {
  if (length == 1) {
    // int, float, or string
    if (p_head->type == TOK_STRING) {
      return AstNode_new(p_head->val, OP_STRING, p_head->lineNumber);
    } else if (p_head->type == TOK_FLOAT) {
      return AstNode_new(p_head->val, OP_FLOAT, p_head->lineNumber);
    } else if (p_head->type == TOK_INT) {
      return AstNode_new(p_head->val, OP_INT, p_head->lineNumber);
    } else if (p_head->type == TOK_IDENTIFIER) {
      return AstNode_new(p_head->val, OP_IDENTIFIER, p_head->lineNumber);
    } else {
      printf(
        "Syntax Error @ Line %i: Unexpected %s token.\n", 
        p_head->lineNumber, getTokenTypeString(p_head->type)
      );
      exit(0);
    }
  } else if (length > 1) {
    // application and function
    if (p_head->type == TOK_APPLYOPEN) {
      return parseApplication(p_head, length);
    } else if (p_head->type == TOK_FUNCOPEN) {
      return parseFunction(p_head, length);    
    } else {
      printf(
        "Syntax Error @ Line %i: Unexpected %s token.\n", 
        p_head->lineNumber, getTokenTypeString(p_head->type)
      );
      exit(0);
    }
  }

  return NULL;
}

// parse assignment
// ebnf: assignment = identifier, "=", value;
AstNode *parseAssignment(Token *p_head, int length) {
  if (length < 3) {
    printf(
      "Syntax Error @ Line %i: Incomplete assignment.\n", 
      p_head->lineNumber
    );
    exit(0);

  } else if (p_head->type != TOK_IDENTIFIER) {
    // error handling if first token not identifier
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );
    exit(0); 

  } else if (p_head->p_next->type != TOK_ASSIGNMENT) {
    // error handling if second token not assignment
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->p_next->lineNumber, getTokenTypeString(p_head->p_next->type)
    );
    exit(0); 

  } else {
    // create node and return
    AstNode *res = AstNode_new(NULL, OP_ASSIGNMENT, p_head->lineNumber);
    res->p_headChild = AstNode_new(p_head->val, OP_IDENTIFIER, p_head->lineNumber);
    res->p_headChild->p_next = parseValue(p_head->p_next->p_next, length - 2);

    return res;
  }
}

// parse return
// ebnf: return = "return", value;
AstNode *parseReturn(Token *p_head, int length) {
  if (length < 2) {
    printf(
      "Syntax Error @ Line %i: Incomplete return.\n", 
      p_head->lineNumber
    );
    exit(0);

  } else if (p_head->type != TOK_RETURN) {
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );
    exit(0);

  } else {
    AstNode *res = AstNode_new(NULL, OP_RETURN, p_head->lineNumber);
    res->p_headChild = parseValue(p_head->p_next, length - 1);
    return res;
  }
}

// parse statement
// ebnf: statement = {return | assignment | value};
// precedence: return, assignment, value
AstNode *parseStatement(Token *p_head, int length) {

  // create ast node for statement
  AstNode *res = AstNode_new(NULL, OP_STATEMENT, p_head->lineNumber);
  AstNode *p_lastChild = res->p_headChild;

  // for each token
  Token *p_curr = p_head;
  int i = 0;

  while (p_curr != NULL && i < length) {
    if (p_curr->type == TOK_RETURN) {
      // return case
      // first token of return
      Token *p_returnStart = p_curr; 
      int returnIndex = i;

      if (p_curr->p_next->type == TOK_APPLYOPEN || p_curr->p_next->type == TOK_FUNCOPEN) {

        // go to open apply token
        p_curr = p_curr->p_next;
        i++;

        // skip closure
        // we pass the type + 1 as the close type, as closing types are 1 + the respective opening type
        skipClosure(&i, &p_curr, p_curr->type, p_curr->type + 1, length);

        // add parsed return expression to result
        AstNode_appendChild(res, &p_lastChild, parseReturn(p_returnStart, i - returnIndex + 1));

      } else {
        // parse return with next token
        AstNode_appendChild(res, &p_lastChild, parseReturn(p_returnStart, 2));

        // increment
        i++;
        p_curr = p_curr->p_next;
      }
    } else if (p_curr->type == TOK_IDENTIFIER && p_curr->p_next->type == TOK_ASSIGNMENT) {
      // assignment case
      // first token of assignment
      Token *p_assignmentStart = p_curr;
      int assignmentIndex = i;

      // increment step by 1
      p_curr = p_curr->p_next;
      i++;
      
      // if closure
      if (p_curr->p_next->type == TOK_APPLYOPEN || p_curr->p_next->type == TOK_FUNCOPEN) {

        // go to open apply token
        p_curr = p_curr->p_next;
        i++;

        // skip closure
        // we pass the type + 1 as the close type, as closing types are 1 + the respective opening type
        skipClosure(&i, &p_curr, p_curr->type, p_curr->type + 1, length);

        // add parsed return expression to result
        AstNode_appendChild(res, &p_lastChild, parseAssignment(p_assignmentStart, i - assignmentIndex + 1));

      } else {
        // do assignment with next token
        AstNode_appendChild(res, &p_lastChild, parseAssignment(p_assignmentStart, 3));

        // increment
        i++;
        p_curr = p_curr->p_next;
      }
    } else if (p_curr->type == TOK_APPLYOPEN || p_curr->type == TOK_FUNCOPEN) {
      // case of value which is closure
      // record first token
      Token *p_start = p_curr;
      int startIndex = i;
     
      // skip closure
      skipClosure(&i, &p_curr, p_curr->type, p_curr->type + 1, length);

      // add child
      AstNode_appendChild(res, &p_lastChild, parseValue(p_start, i - startIndex + 1));
    } else {
      // case of value not in closure
      AstNode_appendChild(res, &p_lastChild, parseValue(p_curr, 1));
    }

    i++;
    p_curr = p_curr->p_next;
  }

  return res;
}

// parse function
// ebnf: function = "{", [{identifier, ","}, identifier, "->"], statement, "}";
AstNode *parseFunction(Token *p_head, int length) {

  // if first token is not {, return error
  if (p_head->type != TOK_FUNCOPEN) {
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );

    exit(0);
  }

  // create res
  AstNode *res = AstNode_new(NULL, OP_FUNCTION, p_head->lineNumber);
  AstNode *p_lastChild = NULL;

  // skip to end of function or arrow
  Token *p_curr = p_head;
  int i = 0;

  while (p_curr->type != TOK_ARROW && i < length - 1) {
    p_curr = p_curr->p_next;
    if (p_curr->type == TOK_FUNCOPEN) skipClosure(&i, &p_curr, TOK_FUNCOPEN, TOK_FUNCCLOSE, length);
    i++;
  }

  if (p_curr->type == TOK_FUNCCLOSE) {
    // case where there are no arguments
    res->p_headChild = parseStatement(p_head->p_next, length - 2);
  } else if (p_curr->type == TOK_ARROW) {
    // case where there are arguments
    // go to first argument
    Token *p_curr = p_head->p_next;

    while (p_curr->type != TOK_ARROW) {
      if (p_curr->type != TOK_IDENTIFIER) {
        // error handling in case identifier not found
        printf(
          "Syntax Error @ Line %i: Unexpected %s token.\n", 
          p_curr->lineNumber, getTokenTypeString(p_curr->type)
        );

        exit(0);
      }

      // add identifier
      AstNode_appendChild(res, &p_lastChild, AstNode_new(p_curr->val, OP_IDENTIFIER, p_curr->lineNumber));
      p_curr = p_curr->p_next;
    }

    // add statement
    AstNode_appendChild(res, &p_lastChild, parseStatement(p_curr->p_next, length - i - 2));
    
  } else {
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_curr->lineNumber, getTokenTypeString(p_curr->type)
    );

    exit(0);
  }

  return res;
}

// parse program
// ebnf: program = start, statement, end;
AstNode *parseProgram(Token *p_head, int length) {
  if (p_head->type != TOK_START) {
    printf("Syntax Error @ Line 1: Missing start token.\n");
    exit(0);
  }

  Token *p_curr = p_head;
  for (int i = 0; i < length - 1; i++) p_curr = p_curr->p_next;
  
  if(p_curr->type != TOK_END) {
    printf("Syntax Error @ Line %i: Missing start token.\n", p_curr->lineNumber);
    exit(0);
  }

  return parseStatement(p_head->p_next, length - 2);
}