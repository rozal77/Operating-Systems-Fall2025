// list/list.c
// 
// Implementation for linked list.
//
// <Author>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

// Allocate memory for a new list and initialize head for it
list_t *list_alloc() { 
  list_t* mylist =  (list_t *) malloc(sizeof(list_t)); 
  mylist->head = NULL;
}

// Free all memory that's associated with the list 
void list_free(list_t *l) {
  node_t *current = l -> head;
  node_t *next_node;

  // Iterating through the list, freeing each node
  while(current != NULL){
    next_node = current -> next;
    free(current);
    current = next_node;
  }

  // Resetting the head pointer to NULL
  l -> head = NULL;
}

// Printing the entire list from starting node
void list_print(list_t *l) {
  node_t *temp = l -> head;

  // Loop through each node and print each node's value
  while(temp != NULL){
    printf("%d ", temp -> value); // Printing current node's value
    temp = temp -> next;
  }

  printf("\n");
}

char * listToString(list_t *l) {
  char* buf = (char *) malloc(sizeof(char) * 10024);
  char tbuf[20];

	node_t* curr = l->head;
  while (curr != NULL) {
    sprintf(tbuf, "%d->", curr->value);
    curr = curr->next;
    strcat(buf, tbuf);
  }
  strcat(buf, "NULL");
  return buf;
}

int list_length(list_t *l) { 
  int length = 0;
  node_t *temp = l -> head;

  // Traverse the list and count each node 
  while(temp != NULL){
    length++;
    temp = temp -> next;
  }

  return length;
 }

void list_add_to_back(list_t *l, elem value) {
  node_t *cur_node = (node_t *) getNode(value);
  cur_node -> next = NULL;

  // if list is empty, set the new node as the head
  if(l->head == NULL){
    l->head = cur_node;
    return;
  }

  // Find the last node and add the new one at the end
  node_t *temp = l -> head;
  while(temp -> next != NULL){
    temp = temp -> next;
  }

  temp -> next = cur_node;
}


void list_add_to_front(list_t *l, elem value) {
     node_t *cur_node = (node_t *) getNode(value);

     /* Insert to front */
     node_t *head = l->head;  // get head of list

     cur_node->next = head;
     l->head = cur_node;
}

node_t * getNode(elem value) {
  node_t *mynode;

  mynode = (node_t *) malloc(sizeof(node_t));
  mynode->value = value;
  mynode->next = NULL;

  return mynode;
}

void list_add_at_index(list_t *l, elem value, int index) {
  int length = list_length(l);

  // The index musr be within the valid range
  if(index < 1 || index > length+1){
    return;
  }

  node_t *cur_node = (node_t *) getNode(value);

  // Insertion at the front 
  if(index == 1){
    cur_node -> next = l -> head;
    l -> head = cur_node;
    return;
  }

  // find the node at indexx - 1 and insert the node after it
  node_t *temp = l -> head;
  int count = 1;

  while(count < index - 1){
    temp = temp -> next;
    count++;
  }

  cur_node -> next = temp -> next;
  temp -> next = cur_node;
}

elem list_remove_from_back(list_t *l) {
  if(l -> head == NULL){
    printf("List is empty\n");
    return -1;
  }

  node_t *temp = l -> head;
  elem value;

  if(temp -> next == NULL){
    value = temp -> value;
    free(temp);
    l -> head = NULL;
    return value;
  }

  while(temp->next->next != NULL){
    temp = temp -> next;
  }
  node_t *last_node = temp -> next;
  value = last_node -> value;
  free(last_node);
  temp->next = NULL;

  return value;
 }
elem list_remove_from_front(list_t *l) { 
  if(l -> head == NULL){
    printf("List is empty\n");
    return -1;
  }

  node_t * temp = l -> head;
  elem value = temp -> value;

  l-> head = temp -> next;
  free(temp);

  return value;
}
elem list_remove_at_index(list_t *l, int index) { 
  int length = list_length(l);

  // Ensuring the index is valid
  if(index < 1 || index > length){
    return -1;
  }

  // Case for removing front node
  if(index == 1){
    return list_remove_from_front(l);
  }

  // Traverse to the node before the one to remove 
  node_t *temp = l -> head;
  int count = 1;

  while(count < index - 1){
    temp = temp -> next;
    count++;
  }

  node_t *node_to_remove = temp -> next;
  elem value = node_to_remove -> value;

  temp -> next = node_to_remove -> next;
  free(node_to_remove);

  return value;
}

bool list_is_in(list_t *l, elem value) { 
  node_t *temp = l -> head;

  // Traverse the list to find the value
  while(temp != NULL){
    if(temp -> value == value){
      return true;
    }
    temp = temp -> next;
  }

  return false;
}

elem list_get_elem_at(list_t *l, int index) { 
  int length = list_length(l);

  if(index < 1 || index > length){
    printf("Index is out of bounds\n");
    return -1;
  }

  node_t *temp = l -> head;
  int count = 1;

  while(count < index){
    temp = temp -> next;
    count++;
  }

  return temp -> value;
 }

// Return the index of a. value in the list and -1 if not found
int list_get_index_of(list_t *l, elem value) { 
  node_t *temp = l -> head;
  int index = 1;

  while(temp != NULL){
    if(temp -> value == value){
      return index;
    }
    temp = temp -> next;
    index++;
  }

  return -1;
}

