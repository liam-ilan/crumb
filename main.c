#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

// types for tokens
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

// types for ast nodes (opcodes)
enum Opcodes {
  OP_INT,
  OP_FLOAT,
  OP_STRING,
  OP_IDENTIFIER,
  OP_ASSIGNMENT,
  OP_RETURN,
  OP_STATEMENT,
  OP_APPLICATION,
  OP_FUNCTION
};

// token
// linked list element containing pointer to next token, type of token, and value
typedef struct Token {
  char *val;
  enum TokenType type;
  struct Token *p_next;
  int lineNumber;
} Token;

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

// node in ast
// each node is an element in a linked list of it's siblings
// additionally, each node contains a pointer to a "head" child node (may be null)
// each node has an opCode to designate an opperation when the tree is traversed afterwards (string)
// each node has a val (string)
typedef struct AstNode {
  struct AstNode *p_headChild;
  struct AstNode *p_next;
  enum Opcodes opcode;
  char *val;
} AstNode;

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
  }
}

// print ast nicely
void AstNode_print(AstNode *p_head, int depth) {

  // print current node
  if (p_head->val == NULL) printf("%s\n", getOpcodeString(p_head->opcode));
  else printf("%s: %s\n", getOpcodeString(p_head->opcode), p_head->val);

  // for each child
  AstNode *p_curr = p_head->p_headChild;
  while (p_curr != NULL) {
    
    // create appropriate whitespace and dash
    for (int x = 0; x < depth + 1; x++) printf("  ");
    printf("- ");

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
AstNode* AstNode_new(char* val, enum Opcodes opcode) {
  AstNode *res = (AstNode *) malloc(sizeof(AstNode));
  res->opcode = opcode;
  res->p_headChild = NULL;
  res->p_next = NULL;
  res->val = val;
}

// signatures for parsing functions
AstNode *parseValue(Token *, int);
AstNode *parseAssignment(Token *, int);
AstNode *parseStatement(Token *, int);
AstNode *parseApplication(Token *, int);
AstNode *parseReturn(Token *, int);
AstNode *parseFunction(Token *, int);

// skips closure, such as function or application
// particulary useful for parsing statement
// p_p_curr is the pointer to the pointer to then token containing the open of the closure ("(" or "{")
// open and close are the respective token types designating the open and close types
// p_index is a pointer to the loop itterating over the tokens
// length is the length of the parent expression
void skipClosure(int *p_index, Token **p_p_curr, enum TokenType open, enum TokenType close, int length) {
  int depth = 1;

  // until depth == 0 (we are out of the application), loop
  while (depth != 0 && *p_index < length) {
    *p_p_curr = (*p_p_curr)->p_next;
    (*p_index)++;

    if ((*p_p_curr)->type == close) depth--;
    else if ((*p_p_curr)->type == open) depth++;
  }
}

// parse application
// ebnf: application = "(", {value}, ")";
AstNode *parseApplication(Token *p_head, int length) {
  if (p_head->type != APPLYOPEN) {
    // error handling for invalid first token
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );
    exit(0);
  }

  AstNode *res = AstNode_new(NULL, OP_APPLICATION);
  AstNode *p_lastChild = NULL;

  // go to first token of first item item in application
  Token *p_curr = p_head->p_next;
  int i = 1;

  while(p_curr->type != APPLYCLOSE && i < length) {

    if (p_curr->type == APPLYOPEN || p_curr->type == FUNCOPEN) {
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
  
  if (p_curr->type != APPLYCLOSE) {
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
    if (p_head->type == STRING) {
      return AstNode_new(p_head->val, OP_STRING);
    } else if (p_head->type == FLOAT) {
      return AstNode_new(p_head->val, OP_FLOAT);
    } else if (p_head->type == INT) {
      return AstNode_new(p_head->val, OP_INT);
    } else if (p_head->type == IDENTIFIER) {
      return AstNode_new(p_head->val, OP_IDENTIFIER);
    } else {
      printf(
        "Syntax Error @ Line %i: Unexpected %s token.\n", 
        p_head->lineNumber, getTokenTypeString(p_head->type)
      );
      exit(0);
    }
  } else if (length > 1) {
    // application and function
    if (p_head->type == APPLYOPEN) {
      return parseApplication(p_head, length);
    } else if (p_head->type == FUNCOPEN) {
      return parseFunction(p_head, length);    
    } else {
      printf(
        "Syntax Error @ Line %i: Unexpected %s token.\n", 
        p_head->lineNumber, getTokenTypeString(p_head->type)
      );
      exit(0);
    }
  }
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

  } else if (p_head->type != IDENTIFIER) {
    // error handling if first token not identifier
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );
    exit(0); 

  } else if (p_head->p_next->type != ASSIGNMENT) {
    // error handling if second token not assignment
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->p_next->lineNumber, getTokenTypeString(p_head->p_next->type)
    );
    exit(0); 

  } else {
    // create node and return
    AstNode *res = AstNode_new(NULL, OP_ASSIGNMENT);
    res->p_headChild = AstNode_new(p_head->val, OP_IDENTIFIER);
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

  } else if (p_head->type != RETURN) {
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );
    exit(0);

  } else {
    AstNode *res = AstNode_new(NULL, OP_RETURN);
    res->p_headChild = parseValue(p_head->p_next, length - 1);
    return res;
  }
}

// parse statement
// ebnf: statement = {return | assignment | value};
// precedence: return, assignment, value
AstNode *parseStatement(Token *p_head, int length) {

  // create ast node for statement
  AstNode *res = AstNode_new(NULL, OP_STATEMENT);
  AstNode *p_lastChild = res->p_headChild;

  // for each token
  Token *p_curr = p_head;
  int i = 0;

  while (p_curr != NULL && i < length) {
    if (p_curr->type == RETURN) {
      // return case
      // first token of return
      Token *p_returnStart = p_curr; 
      int returnIndex = i;

      if (p_curr->p_next->type == APPLYOPEN || p_curr->p_next->type == FUNCOPEN) {

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
    } else if (p_curr->type == IDENTIFIER && p_curr->p_next->type == ASSIGNMENT) {
      // assignment case
      // first token of assignment
      Token *p_assignmentStart = p_curr;
      int assignmentIndex = i;

      // increment step by 1
      p_curr = p_curr->p_next;
      i++;
      
      // if closure
      if (p_curr->p_next->type == APPLYOPEN || p_curr->p_next->type == FUNCOPEN) {

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
    } else if (p_curr->type == APPLYOPEN || p_curr->type == FUNCOPEN) {
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
  if (p_head->type != FUNCOPEN) {
    printf(
      "Syntax Error @ Line %i: Unexpected %s token.\n", 
      p_head->lineNumber, getTokenTypeString(p_head->type)
    );

    exit(0);
  }

  // create res
  AstNode *res = AstNode_new(NULL, OP_FUNCTION);
  AstNode *p_lastChild = NULL;

  // skip to end of function or arrow
  Token *p_curr = p_head;
  int i = 0;

  while (p_curr->type != ARROW && i < length - 1) {
    p_curr = p_curr->p_next;
    i++;
  }

  if (p_curr->type == FUNCCLOSE) {
    // case where there are no arguments
    res->p_headChild = parseStatement(p_head->p_next, length - 2);
  } else if (p_curr->type == ARROW) {
    // case where there are arguments
    // go to first argument
    Token *p_curr = p_head->p_next;

    while (p_curr->type != ARROW) {
      if (p_curr->type != IDENTIFIER) {
        // error handling in case identifier not found
        printf(
          "Syntax Error @ Line %i: Unexpected %s token.\n", 
          p_curr->lineNumber, getTokenTypeString(p_curr->type)
        );

        exit(0);
      }

      // add identifier
      AstNode_appendChild(res, &p_lastChild, AstNode_new(p_curr->val, OP_IDENTIFIER));
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
  if (p_head->type != START) {
    printf("Syntax Error @ Line 1: Missing start token.\n");
    exit(0);
  }

  Token *p_curr = p_head;
  for (int i = 0; i < length - 1; i++) p_curr = p_curr->p_next;
  
  if(p_curr->type != END) {
    printf("Syntax Error @ Line %i: Missing start token.\n", p_curr->lineNumber);
    exit(0);
  }

  return parseStatement(p_head->p_next, length - 2);
}

// runs code given string
void run(char *code, int fileLength) {

  // code label
  printf("CODE\n");

  // print code
  printf("%s\n", code);

  // tokens label
  printf("\nTOKENS\n");

  // token list
  Token headToken = {NULL, START, NULL, 1};

  // token count
  int tokenCount = 1;

  // line number
  int lineNumber = 1;

  // for each char (including terminator, helps us not need to push number tokens if they are last)
  int i = 0;
  while (i < fileLength) {
    char c = code[i];

    if (c == '\n') lineNumber++;

    if (c == '(') {
      Token_push(&headToken, NULL, APPLYOPEN, lineNumber);
      tokenCount++;
    } else if (c == ')') {
      Token_push(&headToken, NULL, APPLYCLOSE, lineNumber);
      tokenCount++;
    } else if (c == '=') {
      Token_push(&headToken, NULL, ASSIGNMENT, lineNumber);
      tokenCount++;
    } else if (c == '{') {
      Token_push(&headToken, NULL, FUNCOPEN, lineNumber);
      tokenCount++;
    } else if (c == '}') {
      Token_push(&headToken, NULL, FUNCCLOSE, lineNumber);
      tokenCount++;
    } else if (c == ',') {
      Token_push(&headToken, NULL, COMMA, lineNumber);
      tokenCount++;
    } else if (c == '-' && code[i + 1] == '>') {
      Token_push(&headToken, NULL, ARROW, lineNumber);
      i++;
      tokenCount++;
    } else if (c == '"') {
      
      // record first char in string
      int stringStart = i + 1;

      // go to first char after quotes
      i++;

      // count to last char in string (last quote)
      while (code[i] != '"') {
        i++;

        // error handling
        if (code[i] == '\n') {
          printf("Syntax Error @ Line %i: Unexpected new line before string closed.\n", lineNumber);
          exit(0);
        }

        if (code[i] == '\0') {
          printf("Syntax Error @ Line %i: Unexpected end of file before string closed.\n", lineNumber);
          exit(0);
        }
      };

      // get substring and add token
      char *val = malloc(i - stringStart + 1);
      strncpy(val, &code[stringStart], i - stringStart);
      val[i - stringStart] = '\0';
      Token_push(&headToken, val, STRING, lineNumber);
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

      if (isFloat) Token_push(&headToken, val, FLOAT, lineNumber);
      else Token_push(&headToken, val, INT, lineNumber);
      tokenCount++;

      // make sure to go back to last char of number
      i--;

    } else if (strchr(" \n\r\t\f\v{}()\"=,", c) == NULL && !(c == '-' && code[i + 1] == '>') ) {
      // case of identifier
      
      int identifierStart = i;

      // while valid identifier char
      while (strchr(" \n\r\t\f\v{}()\"=,", code[i]) == NULL && !(code[i] == '-' && code[i + 1] == '>')) i++;

      // get substring and add token
      char *val = malloc(i - identifierStart + 1);
      strncpy(val, &code[identifierStart], i - identifierStart);
      val[i - identifierStart] = '\0';

      if (strcmp(val, "return") == 0) {
        Token_push(&headToken, NULL, RETURN, lineNumber);
      } else {
        Token_push(&headToken, val, IDENTIFIER, lineNumber);
      }

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

  Token_push(&headToken, NULL, END, lineNumber); 
  tokenCount++;

  // print tokens
  Token_print(&headToken, tokenCount);
  printf("Token Count: %i\n", tokenCount);

  // ast label
  printf("\nAST\n");

  // parse
  AstNode *p_headAstNode = parseProgram(&headToken, tokenCount);

  // print AST
  AstNode_print(p_headAstNode, 0);

  // // free tokens
  // Token *p_tmp = NULL;
  // Token *p_curr = headToken.p_next;
  
  // while (p_curr != NULL) {
  //   p_tmp = p_curr;
  //   p_curr = p_curr->p_next;

  //   // free val (also malloced if int or float, thus must be freed)
  //   if(strcmp(p_tmp->type, "int") == 0 || strcmp(p_tmp->type, "float") == 0) {
  //     free(p_tmp->val);
  //   }
    
  //   free(p_tmp);
  // }

  // // free ast
  // AstNode_free(p_headAstNode);
}

#ifndef __EMSCRIPTEN__
int main(int argc, char *argv[]) {
  
  if (argc < 2) {
    printf("Error: Supply file to read from.\n");
    return 0;
  }

  // open file
  FILE *p_file = fopen(argv[1], "r");

  // go to end, and record position (this will be the length of the file)
  fseek(p_file, 0, SEEK_END);
  long fileLength = ftell(p_file);

  // rewind to start
  rewind(p_file);

  // allocate memory (+1 for 0 terminated string)
  char *code = malloc(fileLength + 1);

  // read file and close
  fread(code, fileLength, 1, p_file);
  fclose(p_file);

  // set terminator to 0
  code[fileLength] = 0;

  run(code, fileLength);

  free(code);
  return 0;
}
#endif