#include <netinet/in.h> //structure for storing address information 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <sys/socket.h> //for socket APIs 
#include <sys/types.h> 
#include <unistd.h>
#include "list.h"

#define PORT 9001
#define ACK "ACK"
  
int main(int argc, char const* argv[]) 
{ 
    int n, val, idx;
    // create server socket similar to what was done in client program 
    int servSockD = socket(AF_INET, SOCK_STREAM, 0); 
  
    // string store data to recv/send to/from client 
    char buf[1024];
    char sbuf[1024]; // Used to store the formatted string to send back
    char* token;

    // define server address 
    struct sockaddr_in servAddr; 
  
    // list
    list_t *mylist;

    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(PORT); 
    servAddr.sin_addr.s_addr = INADDR_ANY; 
  
    // bind socket to the specified IP and port 
    bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)); 
  
    // listen for connections 
    listen(servSockD, 1); 
  
    // integer to hold client socket. 
    int clientSocket = accept(servSockD, NULL, NULL); 

    mylist = list_alloc();  // create the list

    while(1){
        // Read up to 1023 bytes to leave room for the null terminator
        n = recv(clientSocket, buf, sizeof(buf) - 1, 0);

        // Handle client disconnect or receive error gracefully
        if (n <= 0) {
            break;
        }

        buf[n] = '\0';
        token = strtok(buf, " ");
        
        if (token == NULL) continue;
    
        if(strcmp(token,"exit") == 0){
            list_free(mylist);
            close(clientSocket);
            close(servSockD);
            exit(0);
        }
        else if(strcmp(token,"get_length") == 0){
            val = list_length(mylist);
            sprintf(sbuf,"%s%d", "Length = ", val);
        }
        else if(strcmp(token,"add_front") == 0){
            token = strtok(NULL, " ");  
            if (token) {
                val = atoi(token);
                list_add_to_front(mylist,val);
            }
            sprintf(sbuf,"%s", ACK); 
        }
        else if(strcmp(token,"add_back") == 0){
            token = strtok(NULL, " ");  
            if (token) {
                val = atoi(token);
                list_add_to_back(mylist,val);
            }
            sprintf(sbuf,"%s", ACK); 
        }
        else if(strcmp(token,"add_position") == 0){
            token = strtok(NULL, " ");
            if (token) {
                idx = atoi(token);
                token = strtok(NULL, " ");
                if (token) {
                    val = atoi(token);
                    list_add_at_index(mylist, idx, val);
                }
            }
            sprintf(sbuf,"%s", ACK); 
        }
        else if(strcmp(token,"remove_front") == 0){
            val = list_remove_from_front(mylist);
            sprintf(sbuf,"%d", val);
        }
        else if(strcmp(token,"remove_back") == 0){
            val = list_remove_from_back(mylist);
            sprintf(sbuf,"%d", val);
        }
        else if(strcmp(token,"remove_position") == 0){
            token = strtok(NULL, " ");
            if (token) {
                idx = atoi(token);
                val = list_remove_at_index(mylist,idx);
                sprintf(sbuf,"%d", val);
            }
        }
        else if(strcmp(token,"get") == 0){
            token = strtok(NULL, " ");
            if (token) {
                idx = atoi(token);
                val = list_get_elem_at(mylist,idx);
                sprintf(sbuf,"%d", val);
            }
        }
        else if(strcmp(token,"print") == 0){
            char *list_str = listToString(mylist);
            sprintf(sbuf,"%s", list_str);
            free(list_str); // Free the memory to prevent memory leaks
        }
        else {
            sprintf(sbuf, "INVALID COMMAND"); // Handle unrecognized commands
        }

        // Send ONLY the length of the string (+1 for null terminator), NOT the whole buffer
        send(clientSocket, sbuf, strlen(sbuf) + 1, 0);
        
        memset(buf, '\0', 1024);
        memset(sbuf, '\0', 1024);
    }
  
    // Final cleanup in case the loop breaks naturally
    list_free(mylist);
    close(clientSocket);
    close(servSockD);
    return 0; 
}