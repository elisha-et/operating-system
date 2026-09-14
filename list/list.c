// list/list.c
//
// Implementation for linked list.
//
// <Author>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

/*
 * Allocates and initializes an empty linked list.
 */
list_t *list_alloc() {
    list_t *mylist = malloc(sizeof(list_t));

    if (mylist == NULL) {
        return NULL;
    }

    mylist->head = NULL;
    return mylist;
}

/*
 * Frees every node in the list and then frees
 * the list structure itself.
 */
void list_free(list_t *l) {
    if (l == NULL) {
        return;
    }

    node_t *curr = l->head;

    while (curr != NULL) {
        node_t *temp = curr;
        curr = curr->next;
        free(temp);
    }

    free(l);
}

/*
 * Prints the linked list in the form:
 * value -> value -> NULL
 */
void list_print(list_t *l) {
    if (l == NULL) {
        printf("NULL\n");
        return;
    }

    node_t *curr = l->head;

    while (curr != NULL) {
        printf("%d -> ", curr->value);
        curr = curr->next;
    }

    printf("NULL\n");
}

/*
 * Converts the linked list into a dynamically
 * allocated string in the form:
 *
 * value->value->NULL
 *
 * The caller is responsible for freeing the
 * returned string.
 */
char *listToString(list_t *l) {
    size_t capacity = 16;
    size_t length = 0;

    char *buf = malloc(capacity);

    if (buf == NULL) {
        return NULL;
    }

    buf[0] = '\0';

    if (l == NULL) {
        strcpy(buf, "NULL");
        return buf;
    }

    node_t *curr = l->head;

    while (curr != NULL) {
        char tbuf[32];

        int chars_written = snprintf(
            tbuf,
            sizeof(tbuf),
            "%d->",
            curr->value
        );

        if (chars_written < 0) {
            free(buf);
            return NULL;
        }

        size_t needed = length + (size_t)chars_written + 1;

        /*
         * Increase the buffer size if there is not
         * enough room for the next value.
         */
        if (needed > capacity) {
            while (capacity < needed) {
                capacity *= 2;
            }

            char *new_buf = realloc(buf, capacity);

            if (new_buf == NULL) {
                free(buf);
                return NULL;
            }

            buf = new_buf;
        }

        strcat(buf, tbuf);
        length += (size_t)chars_written;

        curr = curr->next;
    }

    /*
     * Make sure there is enough space to append "NULL".
     */
    size_t needed = length + strlen("NULL") + 1;

    if (needed > capacity) {
        char *new_buf = realloc(buf, needed);

        if (new_buf == NULL) {
            free(buf);
            return NULL;
        }

        buf = new_buf;
    }

    strcat(buf, "NULL");

    return buf;
}

/*
 * Returns the number of elements in the list.
 */
int list_length(list_t *l) {
    if (l == NULL) {
        return 0;
    }

    int count = 0;
    node_t *curr = l->head;

    while (curr != NULL) {
        count++;
        curr = curr->next;
    }

    return count;
}

/*
 * Creates and initializes a new node.
 */
node_t *getNode(elem value) {
    node_t *mynode = malloc(sizeof(node_t));

    if (mynode == NULL) {
        return NULL;
    }

    mynode->value = value;
    mynode->next = NULL;

    return mynode;
}

/*
 * Adds a value to the back of the list.
 */
void list_add_to_back(list_t *l, elem value) {
    if (l == NULL) {
        return;
    }

    node_t *new_node = getNode(value);

    if (new_node == NULL) {
        return;
    }

    /*
     * If the list is empty, the new node
     * becomes the head.
     */
    if (l->head == NULL) {
        l->head = new_node;
        return;
    }

    node_t *curr = l->head;

    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = new_node;
}

/*
 * Adds a value to the front of the list.
 */
void list_add_to_front(list_t *l, elem value) {
    if (l == NULL) {
        return;
    }

    node_t *new_node = getNode(value);

    if (new_node == NULL) {
        return;
    }

    new_node->next = l->head;
    l->head = new_node;
}

/*
 * Adds a value at the specified 1-based index.
 *
 * Any index <= 1 adds the value to the front.
 * If the index is beyond the valid insertion
 * position, the list is left unchanged.
 */
void list_add_at_index(list_t *l, elem value, int index) {
    if (l == NULL) {
        return;
    }

    if (index <= 1) {
        list_add_to_front(l, value);
        return;
    }

    node_t *curr = l->head;
    int current_idx = 1;

    /*
     * Move to the node immediately before
     * the requested index.
     */
    while (curr != NULL && current_idx < index - 1) {
        curr = curr->next;
        current_idx++;
    }

    /*
     * Index is beyond the valid insertion range.
     */
    if (curr == NULL) {
        return;
    }

    node_t *new_node = getNode(value);

    if (new_node == NULL) {
        return;
    }

    new_node->next = curr->next;
    curr->next = new_node;
}

/*
 * Removes and returns the last value in the list.
 * Returns -1 if the list is empty or invalid.
 */
elem list_remove_from_back(list_t *l) {
    if (l == NULL || l->head == NULL) {
        return -1;
    }

    /*
     * Special case: only one node exists.
     */
    if (l->head->next == NULL) {
        elem value = l->head->value;

        free(l->head);
        l->head = NULL;

        return value;
    }

    node_t *curr = l->head;

    /*
     * Stop at the second-to-last node.
     */
    while (curr->next->next != NULL) {
        curr = curr->next;
    }

    node_t *temp = curr->next;
    elem value = temp->value;

    curr->next = NULL;
    free(temp);

    return value;
}

/*
 * Removes and returns the first value in the list.
 * Returns -1 if the list is empty or invalid.
 */
elem list_remove_from_front(list_t *l) {
    if (l == NULL || l->head == NULL) {
        return -1;
    }

    node_t *temp = l->head;
    elem value = temp->value;

    l->head = temp->next;
    free(temp);

    return value;
}

/*
 * Removes and returns the value at the specified
 * 1-based index.
 *
 * Returns -1 if the index is invalid.
 */
elem list_remove_at_index(list_t *l, int index) {
    if (l == NULL || l->head == NULL || index < 1) {
        return -1;
    }

    if (index == 1) {
        return list_remove_from_front(l);
    }

    node_t *curr = l->head;
    int current_idx = 1;

    /*
     * Move to the node immediately before
     * the node being removed.
     */
    while (curr->next != NULL &&
           current_idx < index - 1) {

        curr = curr->next;
        current_idx++;
    }

    /*
     * No node exists at the requested index.
     */
    if (curr->next == NULL) {
        return -1;
    }

    node_t *temp = curr->next;
    elem value = temp->value;

    curr->next = temp->next;
    free(temp);

    return value;
}

/*
 * Returns true if the specified value exists
 * in the list.
 */
bool list_is_in(list_t *l, elem value) {
    if (l == NULL) {
        return false;
    }

    node_t *curr = l->head;

    while (curr != NULL) {
        if (curr->value == value) {
            return true;
        }

        curr = curr->next;
    }

    return false;
}

/*
 * Returns the value at the specified 1-based index.
 * Returns -1 if the index does not exist.
 */
elem list_get_elem_at(list_t *l, int index) {
    if (l == NULL || index < 1) {
        return -1;
    }

    node_t *curr = l->head;
    int current_idx = 1;

    while (curr != NULL && current_idx < index) {
        curr = curr->next;
        current_idx++;
    }

    if (curr == NULL) {
        return -1;
    }

    return curr->value;
}

/*
 * Returns the 1-based index of the first occurrence
 * of the specified value.
 *
 * Returns -1 if the value is not found.
 */
int list_get_index_of(list_t *l, elem value) {
    if (l == NULL) {
        return -1;
    }

    node_t *curr = l->head;
    int current_idx = 1;

    while (curr != NULL) {
        if (curr->value == value) {
            return current_idx;
        }

        curr = curr->next;
        current_idx++;
    }

    return -1;
}