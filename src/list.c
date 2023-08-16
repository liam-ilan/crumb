#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "generic.h"

void List_print(List *p_target) {
  printf("[List: ");

  // for each item
  ListNode *p_curr = p_target->p_head;
  while (p_curr != NULL) {

    // print item
    Generic_print(p_curr->p_val);
    if (p_curr->p_next != NULL) printf(", ");
    p_curr = p_curr->p_next;
  }

  printf("]");
}

// copy a given list
List *List_copy(List *p_target) {

  List *res = (List *) malloc(sizeof(List));

  // case of empty list
  if (p_target->p_head == NULL) {
    res->p_head = NULL;
    return res;
  }

  // make first node
  res->p_head = (ListNode *) malloc(sizeof(ListNode));
  res->p_head->p_val = Generic_copy(p_target->p_head->p_val);
  
  // for each node in target
  ListNode *p_last = res->p_head;
  ListNode *p_curr = p_target->p_head->p_next;

  while (p_curr != NULL) {
    // create new item
    ListNode* p_new = (ListNode *) malloc(sizeof(ListNode));
    p_new->p_val = Generic_copy(p_curr->p_val);

    // append to list
    p_last->p_next = p_new;

    // increment
    p_curr = p_curr->p_next;
    p_last = p_last->p_next;
  }

  p_last->p_next = NULL;

  return res;
}

// make a new list struct, given a list of generics
List *List_new(Generic *items[], int length) {
  // edge case
  List *res = (List *) malloc(sizeof(List));
  res->p_head = NULL;

  if (length == 0) return res;

  // create first node
  ListNode *p_head = (ListNode *) malloc(sizeof(ListNode));
  p_head->p_val = Generic_copy(items[0]);
  p_head->p_next = NULL;

  // for each item
  ListNode *p_curr = p_head;
  for (int i = 1; i < length; i++) {

    // create a new node
    ListNode *p_next = (ListNode *) malloc(sizeof(ListNode));
    p_next->p_val = Generic_copy(items[i]);
    p_next->p_next = NULL;

    // add it
    p_curr->p_next = p_next;
    
    // go to next node
    p_curr = p_curr->p_next;
  }


  res->p_head = p_head;

  return res;
}

// get item from list
Generic *List_get(List *p_target, int index) {

  // go to index
  ListNode *p_head = p_target->p_head;
  ListNode *p_curr = p_head;
  for (int i = 0; i < index; i++) p_curr = p_curr->p_next;

  // return copy of generic
  return Generic_copy(p_curr->p_val);
}

// insert item at index
List *List_insert(List *p_target, Generic *p_val, int index) {
  List *res = List_copy(p_target);

  ListNode *p_head = res->p_head;

  // edge case
  if (index == 0) {
    // create new value
    ListNode *p_new = (ListNode *) malloc(sizeof(ListNode));
    p_new->p_val = Generic_copy(p_val);
    p_new->p_next = p_head;

    // set new head
    res->p_head = p_new;
  }

  // loop to 1 before the index
  ListNode *p_curr = p_head;
  for (int i = 0; i < index - 1; i++) p_curr = p_curr->p_next;
  
  // insert
  ListNode *p_new = (ListNode *) malloc(sizeof(ListNode));
  p_new->p_val = Generic_copy(p_val);
  p_new->p_next = p_curr->p_next;
  p_curr->p_next = p_new;

  return res;
}

// delete item from list
List *List_delete(List *p_target, int index) {
  List *res = List_copy(p_target);
  ListNode *p_head = res->p_head;

  if (index == 0) {
    // edge case, simply increment the head, and free
    res->p_head = p_head->p_next;
    Generic_free(p_head->p_val);
    free(p_head);
  } else {

    // go to item before index
    ListNode *p_curr = p_head;
    for (int i = 0; i < index - 1; i++) p_curr = p_curr->p_next;
    
    // store item at index
    ListNode *p_tmp = p_curr->p_next;

    // skip item in list
    p_curr->p_next = p_curr->p_next->p_next;

    // free tmp
    Generic_free(p_tmp->p_val);
    free(p_tmp);
  }

  return res;
}

// free list
void List_free(List *p_target) {
  ListNode* p_curr = p_target->p_head;
  ListNode* p_tmp;

  while (p_curr != NULL) {
    p_tmp = p_curr;
    p_curr = p_curr->p_next;
    Generic_free(p_tmp->p_val);
    free(p_tmp);
  }

  free(p_target);
}

// joins all lists into a single one, and returns
List *List_join(List *lists[], int count) {
  // initialize result
  List *res = List_copy(lists[0]);

  // for each list, itterate on res next item is null, and then add the list to res
  ListNode *p_curr = res->p_head;
  for (int i = 1; i < count; i++) {

    // next list to add
    List *nextList = List_copy(lists[i]);

    // go to null, and add list
    while (p_curr->p_next != NULL) p_curr = p_curr->p_next;
    p_curr->p_next = nextList->p_head;

    // frees List without freeing nodes
    free(nextList);
  }

  return res;
}

// returns the sublist from index1 to index2
List *List_sublist(List *p_target, int index1, int index2) {

  // create copy
  List *res = List_copy(p_target);

  // get to index1
  ListNode *p_curr = res->p_head;
  ListNode *p_tmp;

  for (int i = 0; i < index1; i++) {
    // free items as we go
    p_tmp = p_curr;
    p_curr = p_curr->p_next;

    Generic_free(p_tmp->p_val);
    free(p_tmp);
    p_tmp = NULL;
  };
  
  // set item at index 1 to new head
  res->p_head = p_curr;

  // get to end of sublist
  for (int i = 0; i < index2 - index1 - 1; i++) p_curr = p_curr->p_next;

  // keep track of end
  ListNode *p_end = p_curr;

  // free items to end of list
  p_curr = p_curr->p_next;
  while (p_curr != NULL) {
    p_tmp = p_curr;
    p_curr = p_curr->p_next;

    Generic_free(p_tmp->p_val);
    free(p_tmp);
  }
  
  // make sure list ends with NULL
  p_end->p_next = NULL;
  return res;
}

// set item in list
List *List_set(List *p_target, Generic *p_val, int index) {
  // make copy for result
  List *res = List_copy(p_target);

  // increment to correct node
  ListNode *p_curr = res->p_head;
  for (int i = 0; i < index; i++) p_curr = p_curr->p_next;

  // replace old value with new value, and free old value
  Generic_free(p_curr->p_val);
  p_curr->p_val = Generic_copy(p_val);

  return res;
}

// get length of list
int List_length(List *p_target) {
  ListNode *p_curr = p_target->p_head;
  int length = 0;
  while (p_curr != NULL) {
    p_curr = p_curr->p_next;
    length++;
  }
  return length;
}

// delete multiple items from list from index1 to index2
// delete item from list
List *List_deleteMultiple(List *p_target, int index1, int index2) {
  List *res = List_copy(p_target);
  ListNode *p_head = res->p_head;

  if (index1 == 0) {
    
    ListNode *p_curr = p_head;
    ListNode *p_tmp;

    // edge case, simply increment the head, and free
    for (int i = 0; i < index2; i++) {
      p_tmp = p_curr;
      p_curr = p_curr->p_next;

      Generic_free(p_tmp->p_val);
      free(p_tmp);
    }

    res->p_head = p_curr;

  } else {

    // go to item before index1
    ListNode *p_curr = p_head;
    for (int i = 0; i < index1 - 1; i++) p_curr = p_curr->p_next;
    
    // store this item for later
    ListNode *p_end = p_curr;

    // delete all items after p_end
    p_curr = p_curr->p_next;

    ListNode *p_tmp;
    for (int i = 0; i < index2 - index1; i++) {
      p_tmp = p_curr;
      p_curr = p_curr->p_next;

      Generic_free(p_tmp->p_val);
      free(p_tmp);
    };

    // append both lists, seperated by deleted items, together
    p_end->p_next = p_curr;

  }

  return res;
}