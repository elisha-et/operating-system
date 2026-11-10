#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct node {
   char username[30];
   int socket;
   struct node *next;
};

// Room node structure tracking room name and member nodes
struct room_node {
   char room_name[30];
   struct node *members;
   struct room_node *next;
};

// DM connection structure
struct dm_node {
   int socket1;
   int socket2;
   struct dm_node *next;
};

// User list functions
struct node* insertFirstU(struct node *head, int socket, char *username);
struct node* findU(struct node *head, char* username);
struct node* deleteU(struct node *head, int socket);

// Room list functions
struct room_node* insertRoom(struct room_node *head, char *room_name);
struct room_node* findRoom(struct room_node *head, char *room_name);
void addUserToRoom(struct room_node *room, int socket, char *username);
void removeUserFromRoom(struct room_node *room, int socket);
struct room_node* removeUserFromAllRooms(struct room_node *head, int socket);
struct room_node* removeEmptyRooms(struct room_node *head);

// DM list functions
struct dm_node* addDM(struct dm_node *head, int s1, int s2);
struct dm_node* removeDM(struct dm_node *head, int s1, int s2);
struct dm_node* removeAllDMs(struct dm_node *head, int socket);
bool areConnected(struct dm_node *head, int s1, int s2);

#endif