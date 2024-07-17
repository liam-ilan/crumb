#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "generic.h"

void List_print(List *p_target) {
  printf("[List: ");

  for (int i = 0; i < p_target->len; i += 1) {
    Generic_print(p_target->vals[i]);
    if (i != p_target->len - 1)  printf(", ");
  }

  printf("]");
}

// copy a given list
List *List_copy(List *p_target) {
  return List_new(p_target->vals, p_target->len);
}

// make a new list struct, given a list of generics
List *List_new(Generic **items, int length) {
  List *res = (List *) malloc(sizeof(List));
  res->vals = (Generic **) malloc(sizeof(Generic *) * length);
  res->len = length;
  
  for (int i = 0; i < res->len; i += 1) {
    res->vals[i] = Generic_copy(items[i]);
  }

  return res;
}

// get item from list
Generic *List_get(List *p_target, int index) {
  // return copy of generic
  return Generic_copy(p_target->vals[index]);
}

// insert item at index
List *List_insert(List *p_target, Generic *p_val, int index) {
  List *res = (List *) malloc(sizeof(List));
  res->vals = (Generic **) malloc(sizeof(Generic *) * (p_target->len + 1));
  res->len = p_target->len + 1;
  
  for (int i = 0; i < p_target->len; i += 1) {
    res->vals[i] = Generic_copy(p_target->vals[i]);
  }

  res->vals[res->len - 1] = Generic_copy(p_val);
  return res;
}

// delete item from list
List *List_delete(List *p_target, int index) {
  List *res = (List *) malloc(sizeof(List));
  res->vals = (Generic **) malloc(sizeof(Generic *) * (p_target->len - 1));
  res->len = p_target->len - 1;
  
  for (int i = 0; i < res->len; i += 1) {
    res->vals[i] = Generic_copy(p_target->vals[i]);
  }

  return res;
}

// free list
void List_free(List *p_target) {
  for (int i = 0; i < p_target->len; i += 1) {
    Generic_free(p_target->vals[i]);
  }

  free(p_target->vals);
  free(p_target);
}

// joins all lists into a single one, and returns
List *List_join(List *lists[], int count) {
  List *res = (List *) malloc(sizeof(List));
  res->len = 0;

  for (int i = 0; i < count; i += 1) {
    res->len += lists[i]->len;
  }

  res->vals = (Generic **) malloc(sizeof(Generic *) * res->len);
  
  int i = 0;
  for (int listIndex = 0; listIndex < count; listIndex += 1) {
    for (int itemIndex = 0; itemIndex < lists[listIndex]->len; itemIndex += 1) {
      res->vals[i] = Generic_copy(lists[listIndex]->vals[itemIndex]);
      i += 1;
    }
  }

  return res;
}

// returns the sublist from index1 to index2
List *List_sublist(List *p_target, int index1, int index2) {
  List *res = (List *) malloc(sizeof(List));
  res->vals = (Generic **) malloc(sizeof(Generic *) * (index2 - index1));
  res->len = index2 - index1;
  
  for (int i = index1; i < index2; i += 1) {
    res->vals[i - index1] = Generic_copy(p_target->vals[i]);
  }

  return res;
}

// set item in list
List *List_set(List *p_target, Generic *p_val, int index) {
  List *res = (List *) malloc(sizeof(List));
  res->vals = (Generic **) malloc(sizeof(Generic *) * p_target->len);
  res->len = p_target->len;
  
  for (int i = 0; i < res->len; i += 1) {
    if (i != index) {
      res->vals[i] = Generic_copy(p_target->vals[i]);
    } else {
      res->vals[i] = Generic_copy(p_val);
    }
  }

  return res;
}

// get length of list
int List_length(List *p_target) {
  return p_target->len;
}

// delete multiple items from list from index1 to index2
// delete item from list
List *List_deleteMultiple(List *p_target, int index1, int index2) {
  List *res = (List *) malloc(sizeof(List));
  res->vals = (Generic **) malloc(sizeof(Generic *) * (p_target->len - (index2 - index1)));
  res->len = p_target->len - (index2 - index1);
  
  for (int i = 0; i < index1; i += 1) {
    res->vals[i] = Generic_copy(p_target->vals[i]);
  }

  for (int i = index2; i < res->len; i += 1) {
    res->vals[i - index2 + index1] = Generic_copy(p_target->vals[i]);
  }

  return res;
}

int List_compare(List *p_target1, List *p_target2) {

  // Early check on length.
  if (p_target1->len != p_target2->len) return 0;

  for(int i = 0; i < p_target1->len; i += 1) {
    if (!Generic_is(p_target1->vals[i], p_target2->vals[i])) return 0;
  }

  return 1;
}