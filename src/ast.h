#ifndef AST_H
#define AST_H
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

// prototypes
void AstNode_free(AstNode *);
char* getOpcodeString(enum Opcodes);
void AstNode_print(AstNode *, int);
void AstNode_appendChild(AstNode *, AstNode **, AstNode *);
AstNode* AstNode_new(char*, enum Opcodes);

#endif