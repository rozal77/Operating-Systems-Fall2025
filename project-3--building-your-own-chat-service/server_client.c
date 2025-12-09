#include "server.h"

// External variables
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern User *user_head;
extern Room *room_head;
extern char const *server_MOTD;

char *trimwhitespace(char *str) {
  char *end;
  while(isspace((unsigned char)*str)) str++;
  if(*str == 0)  
    return str;
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';
  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  
   int received;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  
   char tmpbuf[MAXBUFF];  
   char cmd[MAXBUFF], username[MAX_NAME_LEN];
   char *arguments[80];
   const char *delimiters = " \t\n\r";

   send(client , server_MOTD , strlen(server_MOTD) , 0 );

   sprintf(username,"guest%d", client);

   pthread_mutex_lock(&rw_lock);
   addUser(client, username);
   addUserToRoom(username, DEFAULT_ROOM);
   pthread_mutex_unlock(&rw_lock);

   // Main Loop: Read returns > 0 if data received. 
   // It returns 0 if client disconnects gracefully.
   // It returns -1 on error.
   while ((received = read(client , buffer, MAXBUFF)) > 0) {
            buffer[received] = '\0'; 
            strcpy(cmd, buffer);  
            strcpy(sbuffer, buffer);

            arguments[0] = strtok(cmd, delimiters);
            int i = 0;
            while(arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, delimiters);
                if(arguments[i] != NULL)
                    arguments[i] = trimwhitespace(arguments[i]);
            }

            if(arguments[0] == NULL) {
                sprintf(buffer, "\nchat>");
                send(client , buffer , strlen(buffer) , 0 );
                continue;
            }

            if(strcmp(arguments[0], "create") == 0 && arguments[1] != NULL) {
               pthread_mutex_lock(&rw_lock);
               addRoom(arguments[1]);
               pthread_mutex_unlock(&rw_lock);

               sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "join") == 0 && arguments[1] != NULL) {
               pthread_mutex_lock(&rw_lock);
               User *u = findUserBySocket(client);
               if(u && findRoomByName(arguments[1])) {
                  addUserToRoom(u->username, arguments[1]);
                  sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
               } else {
                  sprintf(buffer, "Room '%s' does not exist.\nchat>", arguments[1]);
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "leave") == 0 && arguments[1] != NULL) {
               pthread_mutex_lock(&rw_lock);
               User *u = findUserBySocket(client);
               if(u) {
                  removeUserFromRoom(u->username, arguments[1]);
                  sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
               } else {
                  sprintf(buffer, "User not found.\nchat>");
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "connect") == 0 && arguments[1] != NULL) {
               pthread_mutex_lock(&rw_lock);
               User *u = findUserBySocket(client);
               User *target = findUserByName(arguments[1]);
               if(u && target) {
                  addDirectConnection(u->username, target->username);
                  sprintf(buffer, "Connected (DM) with '%s'.\nchat>", target->username);
               } else {
                  sprintf(buffer, "User '%s' not found.\nchat>", arguments[1]);
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "disconnect") == 0 && arguments[1] != NULL) {
               pthread_mutex_lock(&rw_lock);
               User *u = findUserBySocket(client);
               if(u) {
                  removeDirectConnection(u->username, arguments[1]);
                  sprintf(buffer, "Disconnected from '%s'.\nchat>", arguments[1]);
               } else {
                  sprintf(buffer, "User not found.\nchat>");
               }
               pthread_mutex_unlock(&rw_lock);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "rooms") == 0) {
               pthread_mutex_lock(&mutex);
               numReaders++;
               if(numReaders == 1) pthread_mutex_lock(&rw_lock);
               pthread_mutex_unlock(&mutex);

               listAllRooms(client);

               pthread_mutex_lock(&mutex);
               numReaders--;
               if(numReaders == 0) pthread_mutex_unlock(&rw_lock);
               pthread_mutex_unlock(&mutex);
            }
            else if (strcmp(arguments[0], "users") == 0) {
               pthread_mutex_lock(&mutex);
               numReaders++;
               if(numReaders == 1) pthread_mutex_lock(&rw_lock);
               pthread_mutex_unlock(&mutex);

               listAllUsers(client);

               pthread_mutex_lock(&mutex);
               numReaders--;
               if(numReaders == 0) pthread_mutex_unlock(&rw_lock);
               pthread_mutex_unlock(&mutex);
            }
            else if (strcmp(arguments[0], "login") == 0 && arguments[1] != NULL) {
               pthread_mutex_lock(&rw_lock);
               renameUser(client, arguments[1]);
               pthread_mutex_unlock(&rw_lock);

               sprintf(buffer, "Logged in as '%s'.\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "help") == 0 ) {
                sprintf(buffer, "Commands:\nlogin <username>\ncreate <room>\njoin <room>\nleave <room>\nusers\nrooms\nconnect <user>\ndisconnect <user>\nexit\nchat>");
                send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                break; // Break loop to reach cleanup code
            } 
            else {
                 // Sending a message
                 pthread_mutex_lock(&mutex);
                 numReaders++;
                 if(numReaders == 1) pthread_mutex_lock(&rw_lock);
                 pthread_mutex_unlock(&mutex);

                 User *sender = findUserBySocket(client);

                 // Critical: Do not unlock here yet, we need the sender pointer to be valid
                 // while we format and broadcast.
                 // However, broadcast requires iterating lists. 
                 // Since we have the read lock (via the Reader preference logic), we are safe to read.
                 
                 if(sender == NULL) {
                     // Release lock if sender gone
                     pthread_mutex_lock(&mutex);
                     numReaders--;
                     if(numReaders == 0) pthread_mutex_unlock(&rw_lock);
                     pthread_mutex_unlock(&mutex);

                    sprintf(tmpbuf,"\nchat>");
                    send(client, tmpbuf, strlen(tmpbuf), 0);
                    continue;
                 }

                 sprintf(tmpbuf,"\n::%s> %s\nchat>", sender->username, sbuffer);
                 
                 // Perform the broadcast using the helper function in server.c
                 broadcastMessage(sender, tmpbuf);

                 pthread_mutex_lock(&mutex);
                 numReaders--;
                 if(numReaders == 0) pthread_mutex_unlock(&rw_lock);
                 pthread_mutex_unlock(&mutex);
            }

            memset(buffer, 0, sizeof(buffer));
   }

   // Cleanup code - reached via "exit" command OR socket disconnect (read <= 0)
   pthread_mutex_lock(&rw_lock);
   removeUser(client);
   pthread_mutex_unlock(&rw_lock);
   
   close(client);
   return NULL;
}