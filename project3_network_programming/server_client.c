#include "server.h"

extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern struct room_node *room_head;
extern struct dm_node *dm_head;
extern char *server_MOTD;

char *trimwhitespace(char *str) {
  char *end;
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0) return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}

void reader_lock() {
    pthread_mutex_lock(&mutex);
    numReaders++;
    if(numReaders == 1) {
        pthread_mutex_lock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

void reader_unlock() {
    pthread_mutex_lock(&mutex);
    numReaders--;
    if(numReaders == 0) {
        pthread_mutex_unlock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  
   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  
   char tmpbuf[MAXBUFF];  
   char cmd[MAXBUFF], username[30];
   char *arguments[80];

   send(client, server_MOTD, strlen(server_MOTD), 0); 

   // Assign initial guest username and join default Lobby
   sprintf(username, "guest%d", client);
   
   pthread_mutex_lock(&rw_lock);
   head = insertFirstU(head, client, username);
   struct room_node *lobby = findRoom(room_head, DEFAULT_ROOM);
   if(lobby != NULL) {
       addUserToRoom(lobby, client, username);
   }
   pthread_mutex_unlock(&rw_lock);

   while (1) {
      if ((received = read(client, buffer, MAXBUFF)) > 0) {
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);
         
            arguments[0] = strtok(cmd, delimiters);
            i = 0;
            while(arguments[i] != NULL) {
               arguments[++i] = strtok(NULL, delimiters); 
               if(arguments[i-1] != NULL)
                   strcpy(arguments[i-1], trimwhitespace(arguments[i-1]));
            } 

            if(arguments[0] == NULL) {
                memset(buffer, 0, sizeof(buffer));
                continue;
            }

            if(strcmp(arguments[0], "create") == 0) {
               if(arguments[1] != NULL) {
                   pthread_mutex_lock(&rw_lock);
                   room_head = insertRoom(room_head, arguments[1]);
                   pthread_mutex_unlock(&rw_lock);
               }
               sprintf(buffer, "\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "join") == 0) {
               if(arguments[1] != NULL) {
                   pthread_mutex_lock(&rw_lock);
                   struct room_node *r = findRoom(room_head, arguments[1]);
                   if(r == NULL) {
                       room_head = insertRoom(room_head, arguments[1]);
                       r = findRoom(room_head, arguments[1]);
                   }
                   addUserToRoom(r, client, username);
                   pthread_mutex_unlock(&rw_lock);
               }
               sprintf(buffer, "\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "leave") == 0) {
               if(arguments[1] != NULL) {
                   pthread_mutex_lock(&rw_lock);
                   struct room_node *r = findRoom(room_head, arguments[1]);
                   if(r != NULL) {
                       removeUserFromRoom(r, client);
                       room_head = removeEmptyRooms(room_head);
                   }
                   pthread_mutex_unlock(&rw_lock);
               }
               sprintf(buffer, "\nchat>");
               send(client, buffer, strlen(buffer), 0);
            } 
            else if (strcmp(arguments[0], "connect") == 0) {
               if(arguments[1] != NULL) {
                   pthread_mutex_lock(&rw_lock);
                   struct node *targetUser = findU(head, arguments[1]);
                   if(targetUser != NULL) {
                       dm_head = addDM(dm_head, client, targetUser->socket);
                   }
                   pthread_mutex_unlock(&rw_lock);
               }
               sprintf(buffer, "\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }
            else if (strcmp(arguments[0], "disconnect") == 0) {
               if(arguments[1] != NULL) {
                   pthread_mutex_lock(&rw_lock);
                   struct node *targetUser = findU(head, arguments[1]);
                   if(targetUser != NULL) {
                       dm_head = removeDM(dm_head, client, targetUser->socket);
                   }
                   pthread_mutex_unlock(&rw_lock);
               }
               sprintf(buffer, "\nchat>");
               send(client, buffer, strlen(buffer), 0);
            }                  
            else if (strcmp(arguments[0], "rooms") == 0) {
                reader_lock();
                strcpy(buffer, "\nActive Rooms:\n");
                struct room_node *r = room_head;
                while(r != NULL) {
                    strcat(buffer, "- ");
                    strcat(buffer, r->room_name);
                    strcat(buffer, "\n");
                    r = r->next;
                }
                strcat(buffer, "\nchat>");
                reader_unlock();
                send(client, buffer, strlen(buffer), 0);                            
            }   
            else if (strcmp(arguments[0], "users") == 0) {
                reader_lock();
                strcpy(buffer, "\nConnected Users:\n");
                struct node *u = head;
                while(u != NULL) {
                    strcat(buffer, "- ");
                    strcat(buffer, u->username);
                    strcat(buffer, "\n");
                    u = u->next;
                }
                strcat(buffer, "\nchat>");
                reader_unlock();
                send(client, buffer, strlen(buffer), 0);
            }                           
            else if (strcmp(arguments[0], "login") == 0) {
                if(arguments[1] != NULL) {
                    pthread_mutex_lock(&rw_lock);
                    // Check duplicate username
                    if(findU(head, arguments[1]) != NULL) {
                        sprintf(buffer, "\nError: Username '%s' is already taken.\nchat>", arguments[1]);
                        send(client, buffer, strlen(buffer), 0);
                    } else {
                        // Rename in main user list
                        struct node *curr = head;
                        while(curr != NULL) {
                            if(curr->socket == client) {
                                strcpy(curr->username, arguments[1]);
                                break;
                            }
                            curr = curr->next;
                        }
                        // Rename in all joined rooms
                        struct room_node *r = room_head;
                        while(r != NULL) {
                            struct node *m = r->members;
                            while(m != NULL) {
                                if(m->socket == client) {
                                    strcpy(m->username, arguments[1]);
                                }
                                m = m->next;
                            }
                            r = r->next;
                        }
                        strcpy(username, arguments[1]);
                        sprintf(buffer, "\nchat>");
                        send(client, buffer, strlen(buffer), 0);
                    }
                    pthread_mutex_unlock(&rw_lock);
                } else {
                    sprintf(buffer, "\nUsage: login <username>\nchat>");
                    send(client, buffer, strlen(buffer), 0);
                }
            } 
            else if (strcmp(arguments[0], "help") == 0) {
                sprintf(buffer, "login <username> - \"login with username\" \ncreate <room> - \"create a room\" \njoin <room> - \"join a room\" \nleave <room> - \"leave a room\" \nusers - \"list all users\" \nrooms - \"list all rooms\" \nconnect <user> - \"connect to user\" \ndisconnect <user> - \"disconnect from user\" \nexit - \"exit chat\" \n\nchat>");
                send(client, buffer, strlen(buffer), 0); 
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                pthread_mutex_lock(&rw_lock);
                head = deleteU(head, client);
                room_head = removeUserFromAllRooms(room_head, client);
                room_head = removeEmptyRooms(room_head);
                dm_head = removeAllDMs(dm_head, client);
                pthread_mutex_unlock(&rw_lock);
                
                close(client);
                pthread_exit(NULL);
            }                         
            else { 
                 // Selective Broadcast: send only to shared rooms or direct connections
                 sprintf(tmpbuf, "\n::%s> %s\nchat>", username, sbuffer);
                 strcpy(sbuffer, tmpbuf);
                       
                 reader_lock();
                 struct node *currentUser = head;
                 while(currentUser != NULL) {
                     if(client != currentUser->socket) {  
                         bool shouldSend = false;

                         // Room membership check
                         struct room_node *r = room_head;
                         while(r != NULL) {
                             if(findU(r->members, username) != NULL && findU(r->members, currentUser->username) != NULL) {
                                 shouldSend = true;
                                 break;
                             }
                             r = r->next;
                         }

                         // DM check
                         if(!shouldSend && areConnected(dm_head, client, currentUser->socket)) {
                             shouldSend = true;
                         }

                         if(shouldSend) {
                             send(currentUser->socket, sbuffer, strlen(sbuffer), 0); 
                         }
                     }
                     currentUser = currentUser->next;
                 }
                 reader_unlock();
            }
         memset(buffer, 0, sizeof(buffer));
      } else {
          // Client disconnected abruptly
          pthread_mutex_lock(&rw_lock);
          head = deleteU(head, client);
          room_head = removeUserFromAllRooms(room_head, client);
          room_head = removeEmptyRooms(room_head);
          dm_head = removeAllDMs(dm_head, client);
          pthread_mutex_unlock(&rw_lock);
          close(client);
          pthread_exit(NULL);
      }
   }
   return NULL;
}