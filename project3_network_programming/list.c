#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"

// Insert user at the top of the list
struct node* insertFirstU(struct node *head, int socket, char *username) {
    if(findU(head, username) == NULL) {
       struct node *link = (struct node*) malloc(sizeof(struct node));
       link->socket = socket;
       strcpy(link->username, username);
       link->next = head;
       head = link;
   } else {
       printf("Duplicate user registration attempt: %s\n", username);
   }
   return head;
}

// Find user by username
struct node* findU(struct node *head, char* username) {
   struct node* current = head;
   if(head == NULL) return NULL;
   while(current != NULL) {
      if(strcmp(current->username, username) == 0) {
         return current;
      }
      current = current->next;
   }      
   return NULL;
}

// Delete user by socket descriptor
struct node* deleteU(struct node *head, int socket) {
    struct node *current = head;
    struct node *prev = NULL;
    if (head == NULL) return NULL;

    while (current != NULL) {
        if (current->socket == socket) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return head;
        }
        prev = current;
        current = current->next;
    }
    return head;
}

// Insert a new room if it does not exist
struct room_node* insertRoom(struct room_node *head, char *room_name) {
    if(findRoom(head, room_name) == NULL) {
        struct room_node *link = (struct room_node*) malloc(sizeof(struct room_node));
        strcpy(link->room_name, room_name);
        link->members = NULL;
        link->next = head;
        head = link;
    }
    return head;
}

// Find room by name
struct room_node* findRoom(struct room_node *head, char *room_name) {
    struct room_node *current = head;
    while(current != NULL) {
        if(strcmp(current->room_name, room_name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Add user to room's member list
void addUserToRoom(struct room_node *room, int socket, char *username) {
    if (room != NULL) {
        struct node *curr = room->members;
        while (curr != NULL) {
            if (curr->socket == socket) return; // already in room
            curr = curr->next;
        }
        room->members = insertFirstU(room->members, socket, username);
    }
}

// Remove user from a room
void removeUserFromRoom(struct room_node *room, int socket) {
    if (room != NULL) {
        room->members = deleteU(room->members, socket);
    }
}

// Remove user from all room member lists
struct room_node* removeUserFromAllRooms(struct room_node *head, int socket) {
    struct room_node *curr = head;
    while (curr != NULL) {
        curr->members = deleteU(curr->members, socket);
        curr = curr->next;
    }
    return head;
}

// Automatically purge non-Lobby empty rooms
struct room_node* removeEmptyRooms(struct room_node *head) {
    struct room_node *curr = head;
    struct room_node *prev = NULL;
    
    while(curr != NULL) {
        if(curr->members == NULL && strcmp(curr->room_name, "Lobby") != 0) {
            struct room_node *temp = curr;
            if(prev == NULL) {
                head = curr->next;
                curr = head;
            } else {
                prev->next = curr->next;
                curr = curr->next;
            }
            free(temp);
            continue;
        }
        prev = curr;
        curr = curr->next;
    }
    return head;
}

// Direct Messaging (DM) tracking
struct dm_node* addDM(struct dm_node *head, int s1, int s2) {
    if (!areConnected(head, s1, s2) && s1 != s2) {
        struct dm_node *link = (struct dm_node*) malloc(sizeof(struct dm_node));
        link->socket1 = s1;
        link->socket2 = s2;
        link->next = head;
        head = link;
    }
    return head;
}

struct dm_node* removeDM(struct dm_node *head, int s1, int s2) {
    struct dm_node *curr = head;
    struct dm_node *prev = NULL;
    while (curr != NULL) {
        if ((curr->socket1 == s1 && curr->socket2 == s2) || (curr->socket1 == s2 && curr->socket2 == s1)) {
            if (prev == NULL) head = curr->next;
            else prev->next = curr->next;
            free(curr);
            return head;
        }
        prev = curr;
        curr = curr->next;
    }
    return head;
}

struct dm_node* removeAllDMs(struct dm_node *head, int socket) {
    struct dm_node *curr = head;
    while (curr != NULL) {
        struct dm_node *next_node = curr->next;
        if (curr->socket1 == socket || curr->socket2 == socket) {
            head = removeDM(head, curr->socket1, curr->socket2);
        }
        curr = next_node;
    }
    return head;
}

bool areConnected(struct dm_node *head, int s1, int s2) {
    struct dm_node *curr = head;
    while (curr != NULL) {
        if ((curr->socket1 == s1 && curr->socket2 == s2) || (curr->socket1 == s2 && curr->socket2 == s1)) {
            return true;
        }
        curr = curr->next;
    }
    return false;
}