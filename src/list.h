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
  ListNode *p_head;
} List;

// prototypes
void List_print(List *);
List *List_new(Generic **, int);
List *List_copy(List *);
Generic *List_get(List *, int);
List *List_put(List *, Generic *, int);
List *List_delete(List *, int);
void List_free(List *);

#endif