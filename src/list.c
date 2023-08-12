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
  if (length == 0) return NULL;

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


  List *res = (List *) malloc(sizeof(List));
  res->p_head = p_head;

  return res;
}

Generic *List_get(List *p_target, int index) {

  // go to index
  ListNode *p_head = p_target->p_head;
  ListNode *p_curr = p_head;
  for (int i = 0; i < index; i++) p_curr = p_curr->p_next;

  // return copy of generic
  return Generic_copy(p_curr->p_val);
}

// put item at index
List *List_put(List *p_target, Generic *p_val, int index) {
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
