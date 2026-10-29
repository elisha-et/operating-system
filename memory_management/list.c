// list/list.c
// 
// Implementation for linked list.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

list_t *list_alloc() { 
  list_t* list = (list_t*)malloc(sizeof(list_t));
  list->head = NULL;
  return list; 
}

node_t *node_alloc(block_t *blk) {   
  node_t* node = malloc(sizeof(node_t));
  node->next = NULL;
  node->blk = blk;
  return node; 
}

// Iteratively free blocks and nodes to prevent memory leaks
void list_free(list_t *l){
  node_t *current = l->head;
  node_t *next;
  while(current != NULL) {
      next = current->next;
      if(current->blk != NULL) {
          free(current->blk);
      }
      free(current);
      current = next;
  }
  free(l);
}

void node_free(node_t *node){
  free(node);
}

void list_print(list_t *l) {
  node_t *current = l->head;
  block_t *b;
  
  if (current == NULL){
    printf("list is empty\n");
  }
  while (current != NULL){
    b = current->blk;
    printf("PID=%d START:%d END:%d", b->pid, b->start, b->end);
    current = current->next;
  }
}

int list_length(list_t *l) { 
  node_t *current = l->head;
  int i = 0;
  while (current != NULL){
    i++;
    current = current->next;
  }
  return i; 
}

void list_add_to_back(list_t *l, block_t *blk){  
  node_t* newNode = node_alloc(blk);
  newNode->next = NULL;
  if(l->head == NULL){
    l->head = newNode;
  }
  else{
    node_t *current = l->head;
    while(current->next != NULL){
      current = current->next;
    }
    current->next = newNode;
  }
}

void list_add_to_front(list_t *l, block_t *blk){  
  node_t* newNode = node_alloc(blk);
  newNode->next = l->head;
  l->head = newNode;
}

void list_add_at_index(list_t *l, block_t *blk, int index){
  int i = 0;
  node_t *newNode = node_alloc(blk);
  node_t *current = l->head;

  if(index == 0){ // Insert at the head
    newNode->next = l->head;
    l->head = newNode;
  }
  else if(index > 0){
    // Traverse to the node just before the insertion point
    while(i < index - 1 && current != NULL){
      current = current->next;
      i++;
    }
    if (current != NULL) {
        newNode->next = current->next;
        current->next = newNode;
    } else {
        free(newNode); // Index out of bounds, clean up
    }
  }
}

void list_add_ascending_by_address(list_t *l, block_t *newblk){
    node_t *newNode = node_alloc(newblk);
    
    // If list is empty, insert as the first node
    if (l->head == NULL) {
        l->head = newNode;
        return;
    }
    
    // If the new block comes before the head, update the head
    if (newblk->start < l->head->blk->start) {
        newNode->next = l->head;
        l->head = newNode;
        return;
    }
    
    // Traverse to find the correct insertion point
    node_t *current = l->head;
    while (current->next != NULL && current->next->blk->start < newblk->start) {
        current = current->next;
    }
    
    newNode->next = current->next;
    current->next = newNode;
}

void list_add_ascending_by_blocksize(list_t *l, block_t *newblk){
    node_t *newNode = node_alloc(newblk);
    int newblk_size = newblk->end - newblk->start + 1;
    
    if (l->head == NULL) {
        l->head = newNode;
        return;
    }
    
    int head_size = l->head->blk->end - l->head->blk->start + 1;
    // Insert before head if smaller
    if (newblk_size < head_size) {
        newNode->next = l->head;
        l->head = newNode;
        return;
    }
    
    node_t *current = l->head;
    while (current->next != NULL) {
        int next_size = current->next->blk->end - current->next->blk->start + 1;
        if (newblk_size < next_size) {
            break;
        }
        current = current->next;
    }
    
    newNode->next = current->next;
    current->next = newNode;
}

void list_add_descending_by_blocksize(list_t *l, block_t *blk){
  node_t *current;
  node_t *prev;
  node_t *newNode = node_alloc(blk);
  
  // FIXED: Added +1 to accurately calculate the block size
  int newblk_size = blk->end - blk->start + 1; 
  int curblk_size;
  
  if(l->head == NULL){
    l->head = newNode;
  }
  else{
    prev = current = l->head;
    curblk_size = current->blk->end - current->blk->start + 1;
    
    if(current->next == NULL) { 
       if(newblk_size >= curblk_size) { 
          newNode->next = l->head;
          l->head = newNode;
       }
       else { 
          current->next = newNode;
          newNode->next = NULL;
       }
    }
    else { 
       if(newblk_size >= curblk_size) { 
          newNode->next = l->head;
          l->head = newNode;
       }
       else {
          // Traverse until we find a block smaller than the new one
          while(current != NULL && newblk_size <= curblk_size) {
               prev = current;
               current = current->next;
               if(current != NULL) 
                     // FIXED: Added +1 here as well
                     curblk_size = current->blk->end - current->blk->start + 1; 
          }
          prev->next = newNode;
          newNode->next = current;
       }
    }
  }
}

void list_coalese_nodes(list_t *l){ 
    if (l->head == NULL) {
        return;
    }
    
    node_t *prev = l->head;
    node_t *current = l->head->next;
    
    while (current != NULL) {
        // If the blocks are physically adjacent
        if (prev->blk->end + 1 == current->blk->start) {
            // Expand the previous block to swallow the current block
            prev->blk->end = current->blk->end;
            prev->next = current->next;
            
            // Clean up the swallowed block's memory to avoid leaks
            free(current->blk);
            node_free(current);
            
            current = prev->next;
        } else {
            // Not adjacent, advance pointers
            prev = current;
            current = current->next;
        }
    }
}

block_t* list_remove_from_back(list_t *l){
  block_t *value = NULL;
  node_t *current = l->head;

  if(l->head != NULL){
    if(current->next == NULL) { // Only one node in the list
         l->head = NULL;        // FIXED: Clear the head pointer
         value = current->blk;
         node_free(current);
    }
    else {
         // Traverse to the second-to-last node
         while (current->next->next != NULL){
            current = current->next;
         }
         // FIXED: Grab the value from the last node (current->next), not the current one
         value = current->next->blk;
         node_free(current->next);
         current->next = NULL;
    }
  }
  return value;
}

block_t* list_get_from_front(list_t *l) {
  block_t *value = NULL;
  if(l->head == NULL){
    return value;
  }
  else{
    node_t *current = l->head;
    value = current->blk;
  }
  return value; 
}


block_t* list_remove_from_front(list_t *l) { 
  block_t *value = NULL;
  if(l->head == NULL){
    return value;
  }
  else{
    node_t *current = l->head;
    value = current->blk;
    l->head = l->head->next;
    node_free(current);
  }
  return value; 
}

block_t* list_remove_at_index(list_t *l, int index) { 
  int i;
  block_t* value = NULL;
  bool found = false;
  
  if(l->head == NULL){
    return value;
  }
  else if (index == 0){
    return list_remove_from_front(l);
  }
  else if (index > 0){
    node_t *current = l->head;
    node_t *prev = current;
    
    i = 0;
    while(current != NULL && !found){
      if(i == index)
          found = true;
      else {
         prev = current;
         current = current->next;
         i++;
      }
    }
    
    if(found) {
      value = current->blk; 
      prev->next = current->next;
      node_free(current);
    }
  }
  return value; 
}

bool compareBlks(block_t* a, block_t *b) {
  if(a->pid == b->pid && a->start == b->start && a->end == b->end)
     return true;
  return false;
}

bool compareSize(int a, block_t *b) { 
  if(a <= (b->end - b->start) + 1)
     return true;
  return false;
}

bool comparePid(int a, block_t *b) {
  if(a == b->pid)
     return true;
  return false;
}

bool list_is_in(list_t *l, block_t* value) { 
  node_t *current = l->head;
  while(current != NULL){
    if(compareBlks(value, current->blk)){
      return true;
    }
    current = current->next;
  }
  return false; 
}

block_t* list_get_elem_at(list_t *l, int index) { 
 int i;
  block_t* value = NULL;
  if(l->head == NULL){
    return value;
  }
  else if (index == 0){
    return list_get_from_front(l);
  }
  else if (index > 0){
    node_t *current = l->head;
    
    i = 0;
    while(current != NULL){
      if(i == index)
          return(current->blk);
      else {
         current = current->next;
         i++;
      }
    }
  }
  return value; 
}

int list_get_index_of(list_t *l, block_t* value){
 int i = 0;
 node_t *current = l->head;
 if(l->head == NULL){
    return -1;
  }
  
  while (current != NULL){
   if (compareBlks(value,current->blk)){
     return i;
    }
    current = current->next;
    i++;
  }
  return -1; 
}

bool list_is_in_by_size(list_t *l, int Size){ 
  node_t *current = l->head;
  while(current != NULL){
    if(compareSize(Size, current->blk)){
      return true;
    }
    current = current->next;
  }
  return false; 
}

bool list_is_in_by_pid(list_t *l, int pid){ 
  node_t *current = l->head;
  while(current != NULL){
    if(comparePid(pid, current->blk)){
      return true;
    }
    current = current->next;
  }
  return false; 
}

int list_get_index_of_by_Size(list_t *l, int Size){
 int i = 0;
 node_t *current = l->head;
 if(l->head == NULL){
    return -1;
  }
  
  while (current != NULL){
   if (compareSize(Size,current->blk)){
     return i;
    }
    current = current->next;
    i++;
  }
  return -1; 
}
                   
int list_get_index_of_by_Pid(list_t *l, int pid){
 int i = 0;
 node_t *current = l->head;
 if(l->head == NULL){
    return -1;
  }
  
  while (current != NULL){
   if (comparePid(pid,current->blk)){
     return i;
    }
    current = current->next;
    i++;
  }
  return -1; 
}