#ifndef LIST_H
#define LIST_H
#include "generic.h"

// node in a linked list
// lists in crumb are singly linked lists
typedef struct ListNode {
  Generic *p_val;
  struct ListNode *p_next;
} ListNode;

// list container
typedef struct List {
  Generic **vals;
  int len;
} List;

// prototypes
void List_print(List *);
List *List_new(Generic **, int);
List *List_copy(List *);
Generic *List_get(List *, int);
List *List_insert(List *, Generic *, int);
List *List_delete(List *, int);
void List_free(List *);
List *List_join(List **, int);
List *List_sublist(List *, int, int);
int List_length(List *);
List *List_set(List *, Generic *, int);
List *List_deleteMultiple(List *, int, int);
int List_compare(List *, List *);

#endif