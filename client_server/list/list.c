#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

list_t *list_alloc() {
    list_t *list = (list_t *)malloc(sizeof(list_t));
    if (list != NULL) {
        list->head = NULL;
    }
    return list;
}

void list_free(list_t *l) {
    if (l == NULL) return;
    node_t *current = l->head;
    node_t *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(l);
}

void list_add_to_front(list_t *l, int value) {
    if (l == NULL) return;
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) return;
    new_node->value = value;
    new_node->next = l->head;
    l->head = new_node;
}

void list_add_to_back(list_t *l, int value) {
    if (l == NULL) return;
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) return;
    new_node->value = value;
    new_node->next = NULL;

    if (l->head == NULL) {
        l->head = new_node;
        return;
    }

    node_t *current = l->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

void list_add_at_index(list_t *l, int index, int value) {
    if (l == NULL || index < 0) return;
    if (index == 0) {
        list_add_to_front(l, value);
        return;
    }

    node_t *current = l->head;
    for (int i = 0; current != NULL && i < index - 1; i++) {
        current = current->next;
    }

    if (current == NULL) return; // index out of bounds

    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) return;
    new_node->value = value;
    new_node->next = current->next;
    current->next = new_node;
}

int list_remove_from_front(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;
    node_t *temp = l->head;
    int val = temp->value;
    l->head = l->head->next;
    free(temp);
    return val;
}

int list_remove_from_back(list_t *l) {
    if (l == NULL || l->head == NULL) return -1;
    if (l->head->next == NULL) {
        int val = l->head->value;
        free(l->head);
        l->head = NULL;
        return val;
    }

    node_t *current = l->head;
    while (current->next->next != NULL) {
        current = current->next;
    }
    int val = current->next->value;
    free(current->next);
    current->next = NULL;
    return val;
}

int list_remove_at_index(list_t *l, int index) {
    if (l == NULL || l->head == NULL || index < 0) return -1;
    if (index == 0) return list_remove_from_front(l);

    node_t *current = l->head;
    for (int i = 0; current->next != NULL && i < index - 1; i++) {
        current = current->next;
    }

    if (current->next == NULL) return -1; // out of bounds

    node_t *temp = current->next;
    int val = temp->value;
    current->next = temp->next;
    free(temp);
    return val;
}

int list_length(list_t *l) {
    if (l == NULL) return 0;
    int count = 0;
    node_t *current = l->head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

int list_get_elem_at(list_t *l, int index) {
    if (l == NULL || index < 0) return -1;
    node_t *current = l->head;
    for (int i = 0; current != NULL && i < index; i++) {
        current = current->next;
    }
    if (current == NULL) return -1;
    return current->value;
}

char* listToString(list_t *l) {
    char* buf = (char*)malloc(1024);
    if (buf == NULL) return NULL;
    buf[0] = '\0'; 

    if (l == NULL || l->head == NULL) {
        return buf;
    }

    node_t *current = l->head;
    char temp[32];
    while (current != NULL) {
        sprintf(temp, "%d->", current->value);
        strcat(buf, temp);
        current = current->next;
    }
    strcat(buf, "NULL");
    return buf;
}