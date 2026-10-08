#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

/* node struct */
typedef struct node {
    int value;
    struct node *next;
} node_t;

/* list struct */
typedef struct list {
    node_t *head;
} list_t;

/* Functions */
list_t *list_alloc();
void list_free(list_t *l);

void list_add_to_back(list_t *l, int value);
void list_add_to_front(list_t *l, int value);
void list_add_at_index(list_t *l, int index, int value);

int list_remove_from_back(list_t *l);
int list_remove_from_front(list_t *l);
int list_remove_at_index(list_t *l, int index);

int list_get_elem_at(list_t *l, int index);
int list_length(list_t *l);
char* listToString(list_t *l);

#endif