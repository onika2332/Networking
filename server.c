#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "process.h"

#define PORT 5500
#define MAX_PLAYERS 100
#define UP_KEY 'W'
#define DOWN_KEY 'W'

static _Atomic unsigned int cli_count = 0;
// static int uid = 10;

// /* Client structure */
// typedef struct{
// 	struct sockaddr_in address;
// 	int sockfd;
// 	int uid;
// 	char name[32];
// } client_t;

// client_t *clients[MAX_CLIENTS];

// pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

extern node head;
char *room;

/* Add clients to queue */
// void queue_add(client_t *cl){
// 	pthread_mutex_lock(&clients_mutex);

// 	for(int i=0; i < MAX_CLIENTS; ++i){
// 		if(!clients[i]){
// 			clients[i] = cl;
// 			break;
// 		}
// 	}

// 	pthread_mutex_unlock(&clients_mutex);
// }

/* Remove clients to queue */
// void queue_remove(int uid){
// 	pthread_mutex_lock(&clients_mutex);

// 	for(int i=0; i < MAX_CLIENTS; ++i){
// 		if(clients[i]){
// 			if(clients[i]->uid == uid){
// 				clients[i] = NULL;
// 				break;
// 			}
// 		}
// 	}

// 	pthread_mutex_unlock(&clients_mutex);
// }

/* Send message to all clients except sender */
// void send_message(char *s, int uid){
// 	pthread_mutex_lock(&clients_mutex);

// 	for(int i=0; i<MAX_CLIENTS; ++i){
// 		if(clients[i]){
// 			if(clients[i]->uid != uid){
// 				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
// 					perror("ERROR: write to descriptor failed");
// 					break;
// 				}
// 			}
// 		}
// 	}

// 	pthread_mutex_unlock(&clients_mutex);
// }

// char *encode_server(int c_x, int c_y, int b_x, int b_y) {       // c_x, c_y is position of paddles, b_x, b_y is position of ball
//     char client_x[100], client_y[100], ball_x[100], ball_y[100];
//     sprintf(client_x, "%d", c_x);
//     sprintf(client_y, "%d", c_y);
//     sprintf(ball_x, "%d", b_x);
//     sprintf(ball_y, "%d", b_y);
//     char *result = client_x;
//     strcat(result, ",");
//     strcat(result, client_y);
//     strcat(result, ";");
//     strcat(result, ball_x);
//     strcat(result, ",");
//     strcat(result, ball_y);
//     return result;
// }

// int clientX, clientY; // vi tri paddle cua client 

// void decode_server(char data[256]) { 
//     char *tempX, *tempY, dataX[100], dataY[100], *ptr;
//     tempX = strtok(data, ",");
//     tempY = strtok(NULL, ",");
//     strcpy(dataX, tempX);
//     strcpy(dataY, tempY);
//     clientX = strtol(dataX, &ptr, 10);
//     clientY = strtol(dataY, &ptr, 10);
// }

void* client_handler(void* arg) {
    // Read data from file
    FILE *fp;
    char account[100],inPassword[100];
    int status;
    int count = 0;
    fp = fopen("account.txt", "r");

    while(fscanf(fp, "%s %s %d", account, inPassword, &status) != EOF){
        count++;
    }
    fclose(fp);
    fp = fopen("account.txt", "r");
    struct Accounts accounts_list[count + 1];
    for(int i = 0; i < count; i++) {
        fscanf(fp, "%s %s %d", accounts_list[i].account, accounts_list[i].password, &accounts_list[i].status);
        head = addTail(head, accounts_list[i].account, accounts_list[i].password, accounts_list[i].status);
    }
    fclose(fp);
    // end read FILE
    cli_count++;
    int clientfd = (intptr_t) arg;
    pthread_detach(pthread_self());
    char check[50];
    char username[256];
    char password[256];
    char recv_data[256];
    bzero(&recv_data, 12);

    while(1) {
        int data = read(clientfd, &recv_data, 2);
        if(data == 0) break;
        recv_data[data] = '\0';
        printf("Received from client in share-socket %d: %s\n", clientfd, recv_data);
        if(strlen(recv_data) == 0){
            break;
        }
        // Register handle
        if(strcmp(recv_data, "1") == 0){
            printf("Received from client in share-socket %d: Register\n", clientfd);
            data = read(clientfd, &username, 256);
            if(data == 0){
                exit(0);
            }
            username[data] = '\0';
            if (username[strlen(username) - 1] == '\n')
                username[strlen(username) - 1] = 0;
            node checkAcc = findExistAccount(head, username);
            if(checkAcc != NULL) {
                write(clientfd, "NotOK", 6);
            } else {
                write(clientfd, "OK", 3);
                data = read(clientfd, &password, 256);
                if(data == 0){
                    break;
                }
                password[data] = '\0';
                if (password[strlen(password) - 1] == '\n')
                    password[strlen(password) - 1] = 0;
                printf("Receive username from client in share-socket %d: %s\n", clientfd, username);
                printf("Receive password from client in share-socket %d: %s\n", clientfd, password);
                addTail(head, username, password, 1);
                printFile(head);
            }

        } else if(strcmp(recv_data, "2") == 0) {    // Login handle
            printf("Received from client in share-socket %d: Login\n", clientfd);
            
            data = read(clientfd, &username, 256);
            if(data == 0){
                exit(0);
            }
            username[data] = '\0';
            if (username[strlen(username) - 1] == '\n')
                username[strlen(username) - 1] = 0;
            node checkAcc = findExistAccount(head, username);
            if(checkAcc == NULL || checkAcc->status == 0) {
                write(clientfd, "NotOK", 6);
            } else {
                write(clientfd, "OK", 3);
                data = read(clientfd, &password, 256);
                if(data == 0){
                    break;
                }
                password[data] = '\0';
                if (password[strlen(password) - 1] == '\n')
                    password[strlen(password) - 1] = 0;
                printf("Receive username from client in share-socket %d: %s\n", clientfd, username);
                printf("Receive password from client in share-socket %d: %s\n", clientfd, password);
                while(strcmp(checkAcc->password, password) != 0){
                    write(clientfd, "Password not correct!", 21);
                    data = read(clientfd, &password, 256);
                    if(data == 0){
                        break;
                    }
                    password[data] = '\0';
                    if (password[strlen(password) - 1] == '\n')
                    password[strlen(password) - 1] = 0;
                }
                write(clientfd, "CorrectPass", 12);
                back:
                fflush(stdin);
                data = read(clientfd, &recv_data, 2);
                if(data == 0){
                    break;
                }
                recv_data[data] = '\0';
                if(strcmp(recv_data, "1") == 0){    // Waiting room
                    while(1) {
                        data = read(clientfd, &check, 50);
                        if(data == 0) {
                            break;
                        }
                        check[data] = '\0';
                        switch(check[0]) {
                            case 's':
                            case 'S':
                                if(cli_count == 2) {
                                    write(clientfd, "Play", 5);
                                } else {
                                    write(clientfd, "Waiting", 8);
                                }
                                break;
                            case 'q':
                            case 'Q':
                                write(clientfd, "Quit", 5);
                                break;
                            default: 
                                break;
                        }
                    }
                    break;
                } else if(strcmp(recv_data, "2") == 0){ // Change password
                    printf("Received from client in share-socket %d: Change password\n", clientfd);
                    char new[256];
                    data = read(clientfd, &new, 256);
                    if(data == 0) {
                        break;
                    }
                    new[data] = '\0';
                    if (new[strlen(new) - 1] == '\n')
                        new[strlen(new) - 1] = 0;
                    strcpy(checkAcc->password, new);
                    printFile(head);
                    goto back;
                } else if(strcmp(recv_data, "3") == 0){ // Quit
                    printf("Received from client in share-socket %d: Quit game\n", clientfd);
                    break;
                }
            }
        }
    }
}

// Handle Ctrl C
void ctr_c_handler() {
    printf("\nExit Server!\n");
    exit(0);
}

int main(int argc, char *argv[]) {

    int socket_fds[MAX_PLAYERS];     
    struct sockaddr_in socket_addr[MAX_PLAYERS];
    int i;

    // Handle Ctrl C
    signal(SIGINT, ctr_c_handler);

    if(argc != 2) {
        printf("Wrong input format!\n");
        exit(1);
    }
    if(checkValidPort(argv[1]) == 0) {
        printf("Invalid port!\n");
        exit(1);
    }
    int SERV_PORT = atoi(argv[1]);

    if((socket_fds[0] = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
        perror("Error");
        exit(1); 
    }

    bzero((char *) &socket_addr[0], sizeof(socket_addr[0]));  
    socket_addr[0].sin_family = AF_INET;
    socket_addr[0].sin_addr.s_addr = INADDR_ANY; 
    socket_addr[0].sin_port = htons(SERV_PORT);

    if (bind(socket_fds[0], (struct sockaddr *) &socket_addr[0], sizeof(socket_addr[0])) < 0) {
        perror("Error");
        exit(0); 
    }
    
     if(listen(socket_fds[0], 100) != 0) {
        perror("Error");
        exit(0); 
    }

    socklen_t len = sizeof(socket_addr[0]);

    while(1) {
        for(i = 1;; i++){
            socket_fds[i] = accept(socket_fds[0], (struct sockaddr *) &socket_addr[i], &len);
            if (socket_fds[i] < 0) 
                perror("ERROR on accept");
            if(i < 3) { // create thread to handle client request
                pthread_t tid;
                pthread_create(&tid, NULL, &client_handler, (void *)(intptr_t)socket_fds[i]);
            } else {    // if more than 2 clients have access => reject
                close(socket_fds[i]);
                continue;
            }
        }
        close(socket_fds[0]);  
    }
    return 0;
}