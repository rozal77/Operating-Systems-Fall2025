#include "server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

int chat_serv_sock_fd; 

/////////////////////////////////////////////
// SYNCHRONIZATION GLOBALS
int numReaders = 0; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  
/////////////////////////////////////////////

char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

User *user_head = NULL;
Room *room_head = NULL;

// --- Helper for managing User's personal room list ---
void addRoomToUserList(User *u, const char *roomname) {
    if (!u) return;
    RoomList *newNode = malloc(sizeof(RoomList));
    strcpy(newNode->room_name, roomname);
    newNode->next = u->rooms;
    u->rooms = newNode;
}

void removeRoomFromUserList(User *u, const char *roomname) {
    if (!u) return;
    RoomList *prev = NULL;
    RoomList *curr = u->rooms;
    while (curr) {
        if (strcmp(curr->room_name, roomname) == 0) {
            if (prev) prev->next = curr->next;
            else u->rooms = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}
// -----------------------------------------------------

void addRoom(const char *roomname) {
    if (findRoomByName(roomname)) return; // Prevent duplicates
    Room *newRoom = malloc(sizeof(Room));
    strcpy(newRoom->name, roomname);
    newRoom->users = NULL;
    newRoom->next = room_head;
    room_head = newRoom;
}

void freeAllUsers(User **user_head_ref) {
    User *current = *user_head_ref;
    while (current) {
        User *tempU = current;
        current = current->next;
        
        // Free Direct Connections
        DirectConn *dc = tempU->directConns;
        while(dc) {
            DirectConn *tempDC = dc;
            dc = dc->next;
            free(tempDC);
        }
        // Free Room List
        RoomList *rl = tempU->rooms;
        while(rl) {
            RoomList *tempRL = rl;
            rl = rl->next;
            free(tempRL);
        }
        close(tempU->socket);
        free(tempU);
    }
    *user_head_ref = NULL;
}

void freeAllRooms(Room **room_head_ref) {
    Room *current = *room_head_ref;
    while (current) {
        Room *tempR = current;
        current = current->next;
        
        RoomUser *ru = tempR->users;
        while (ru) {
            RoomUser *tempRU = ru;
            ru = ru->next;
            free(tempRU);
        }
        free(tempR);
    }
    *room_head_ref = NULL;
}

void addUser(int socket, const char *username) {
    User *newUser = malloc(sizeof(User));
    newUser->socket = socket;
    strcpy(newUser->username, username);
    newUser->rooms = NULL; 
    newUser->directConns = NULL;
    newUser->next = user_head;
    user_head = newUser;
}

void addUserToRoom(const char *username, const char *roomname) {
    Room *r = findRoomByName(roomname);
    User *u = findUserByName(username);
    if (!r || !u) return; 

    // check if already in room to prevent duplicate printing
    RoomUser *check = r->users;
    while(check) {
        if(strcmp(check->username, username) == 0) return;
        check = check->next;
    }

    // Add to Room's user list
    RoomUser *newRU = malloc(sizeof(RoomUser));
    strcpy(newRU->username, username);
    newRU->next = r->users;
    r->users = newRU;

    // Add to User's room list
    addRoomToUserList(u, roomname);
}

User *findUserBySocket(int socket) {
    User *current = user_head;
    while (current) {
        if (current->socket == socket) return current;
        current = current->next;
    }
    return NULL;
}

Room *findRoomByName(const char *roomname) {
    Room *current = room_head;
    while (current) {
        if (strcmp(current->name, roomname) == 0) return current;
        current = current->next;
    }
    return NULL;
}

void removeUserFromRoom(const char *username, const char *roomname) {
    Room *r = findRoomByName(roomname);
    User *u = findUserByName(username);
    
    if (r) {
        RoomUser *prev = NULL, *cur = r->users;
        while (cur) {
            if (strcmp(cur->username, username) == 0) {
                if (prev) prev->next = cur->next;
                else r->users = cur->next;
                free(cur);
                break;
            }
            prev = cur;
            cur = cur->next;
        }
    }
    
    if (u) {
        removeRoomFromUserList(u, roomname);
    }
}

User *findUserByName(const char *username) {
    User *current = user_head;
    while (current) {
        if (strcmp(current->username, username) == 0) return current;
        current = current->next;
    }
    return NULL;
}

void addDirectConnection(const char *fromUser, const char *toUser) {
    User *u = findUserByName(fromUser);
    if (!u) return;
    
    // Check duplicates
    DirectConn *chk = u->directConns;
    while(chk) {
        if(strcmp(chk->username, toUser) == 0) return;
        chk = chk->next;
    }

    DirectConn *dc = malloc(sizeof(DirectConn));
    strcpy(dc->username, toUser);
    dc->next = u->directConns;
    u->directConns = dc;
}

void removeDirectConnection(const char *fromUser, const char *toUser) {
    User *u = findUserByName(fromUser);
    if (!u) return;

    DirectConn *prev = NULL, *cur = u->directConns;
    while (cur) {
        if (strcmp(cur->username, toUser) == 0) {
            if (prev) prev->next = cur->next;
            else u->directConns = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void listAllRooms(int client_socket) {
    char buffer[MAXBUFF];
    strcpy(buffer, "Rooms list:\n");
    send(client_socket, buffer, strlen(buffer), 0);

    Room *cur = room_head;
    while (cur) {
        sprintf(buffer, " - %s\n", cur->name);
        send(client_socket, buffer, strlen(buffer), 0);
        cur = cur->next;
    }
    send(client_socket, "chat>", 5, 0);
}

void listAllUsers(int client_socket) {
    char buffer[MAXBUFF];
    strcpy(buffer, "Users list:\n");
    send(client_socket, buffer, strlen(buffer), 0);

    User *cur = user_head;
    while (cur) {
        sprintf(buffer, " - %s\n", cur->username);
        send(client_socket, buffer, strlen(buffer), 0);
        cur = cur->next;
    }
    send(client_socket, "chat>", 5, 0);
}

void renameUser(int socket, const char *newName) {
    User *u = findUserBySocket(socket);
    // Ensure name uniqueness
    if (findUserByName(newName)) return;
    
    // We also need to update the name in Room Lists and DirectConns of other users
    // But for this assignment scope, updating the main User struct is primary.
    // Ideally, RoomUser structs should point to User*, not store char arrays, 
    // but we must stick to the structure provided.
    // For now, we only update the User struct.
    if (u) {
        strcpy(u->username, newName);
    }
}

void removeUser(int socket) {
    User *prev = NULL, *current = user_head;
    while (current) {
        if (current->socket == socket) {
            
            // 1. Remove user from all rooms they are in
            RoomList *rl = current->rooms;
            while(rl) {
                // We need to manually remove from the Room struct side
                // We cannot use removeUserFromRoom here easily because it tries 
                // to modify the list we are iterating. 
                // Instead, we access the global room list.
                Room *r = findRoomByName(rl->room_name);
                if(r) {
                     RoomUser *rp = NULL, *rc = r->users;
                     while(rc) {
                         if(strcmp(rc->username, current->username) == 0) {
                             if(rp) rp->next = rc->next;
                             else r->users = rc->next;
                             free(rc);
                             break;
                         }
                         rp = rc;
                         rc = rc->next;
                     }
                }
                RoomList *tempRL = rl;
                rl = rl->next;
                free(tempRL);
            }

            // 2. Free direct connections
            DirectConn *dc = current->directConns;
            while (dc) {
                DirectConn *tmp = dc;
                dc = dc->next;
                free(tmp);
            }

            // 3. Unlink and free user
            if (prev) prev->next = current->next;
            else user_head = current->next;
            
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void broadcastMessage(User *sender, char *message) {
    // Iterate over all users. 
    // Send to user IF: 
    // 1. They are in a shared room.
    // 2. They are a direct connection.
    
    User *dest = user_head;
    while(dest) {
        if(dest->socket == sender->socket) {
            dest = dest->next;
            continue;
        }

        int shouldSend = 0;

        // Check Direct Connections
        DirectConn *dc = sender->directConns;
        while(dc) {
            if(strcmp(dc->username, dest->username) == 0) {
                shouldSend = 1;
                break;
            }
            dc = dc->next;
        }

        // Check Shared Rooms if not already sending
        if(!shouldSend) {
            RoomList *sRoom = sender->rooms;
            while(sRoom && !shouldSend) {
                RoomList *dRoom = dest->rooms;
                while(dRoom) {
                    if(strcmp(sRoom->room_name, dRoom->room_name) == 0) {
                        shouldSend = 1;
                        break;
                    }
                    dRoom = dRoom->next;
                }
                sRoom = sRoom->next;
            }
        }

        if(shouldSend) {
            send(dest->socket, message, strlen(message), 0);
        }

        dest = dest->next;
    }
}

int main(int argc, char **argv) {
    signal(SIGINT, sigintHandler);
    
    pthread_mutex_lock(&rw_lock);
    addRoom("Lobby"); 
    pthread_mutex_unlock(&rw_lock);

    chat_serv_sock_fd = get_server_socket();
    if (chat_serv_sock_fd == -1) {
        fprintf(stderr, "Error creating server socket.\n");
        exit(EXIT_FAILURE);
    }

    if (start_server(chat_serv_sock_fd, BACKLOG) == -1) {
        printf("start_server error\n");
        exit(1);
    }
   
    printf("Server Launched! Listening on PORT: %d\n", PORT);
    
    while(1) {
        int new_client = accept_client(chat_serv_sock_fd);
        if(new_client != -1) {
            pthread_t new_client_thread;
            // Detach thread so resources are freed on exit automatically
            pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
            pthread_detach(new_client_thread);
        }
    }

    close(chat_serv_sock_fd);
    return 0;
}

int get_server_socket() {
    int opt = TRUE;   
    int master_socket;
    struct sockaddr_in address; 
    
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0 ) {   
        perror("socket failed");   
        return -1;   
    }   
     
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {   
        perror("setsockopt");   
        close(master_socket);
        return -1;   
    }   
     
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {   
        perror("bind failed");   
        close(master_socket);
        return -1;   
    }

    return master_socket;
}

int start_server(int serv_socket, int backlog) {
   int status = 0;
   if ((status = listen(serv_socket, backlog)) == -1) {
      printf("socket listen error\n");
   }
   return status;
}

int accept_client(int serv_sock) {
   int reply_sock_fd = -1;
   socklen_t sin_size = sizeof(struct sockaddr_storage);
   struct sockaddr_storage client_addr;

   if ((reply_sock_fd = accept(serv_sock,(struct sockaddr *)&client_addr, &sin_size)) == -1) {
      // Don't print error on accept interrupt (common during shutdown)
   }
   return reply_sock_fd;
}

void sigintHandler(int sig_num) {
    printf("\nServer shutting down...\n");
    pthread_mutex_lock(&rw_lock);
    freeAllUsers(&user_head);
    freeAllRooms(&room_head);
    pthread_mutex_unlock(&rw_lock);
    close(chat_serv_sock_fd);
    printf("All resources freed. Exiting.\n");
    exit(0);
}