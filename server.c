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
#include "paddle.h"
#include "ball.h"
#include "game_action.h"

#define PORT 5500
#define MAX_PLAYERS 100
#define UP_KEY 'W'
#define DOWN_KEY 'W'
#define ROWS 45
#define COLS 110
#define LEFT 0
#define RIGHT 1

struct clientInfo {
    int clientfd; 
    int rivalfd;
    int ready;
    int pos;
};

// char host[256];

typedef struct clientInfo clientInfo;
static _Atomic unsigned int cli_count = 0;
Paddle *left, *right;
Ball* ball;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
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
char room[BUFF_SIZE];

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

char *encode_server(int c_x, int c_y, int b_x, int b_y) {       // c_x, c_y is position of paddles, b_x, b_y is position of ball
    char client_x[100], client_y[100], ball_x[100], ball_y[100];
    sprintf(client_x, "%d", c_x);
    sprintf(client_y, "%d", c_y);
    sprintf(ball_x, "%d", b_x);
    sprintf(ball_y, "%d", b_y);
    char *result = client_x;
    strcat(result, ",");
    strcat(result, client_y);
    strcat(result, ";");
    strcat(result, ball_x);
    strcat(result, ",");
    strcat(result, ball_y);
    return result;
}

int clientX, clientY; // vi tri paddle cua client 

void decode_server(char data[256]) { 
    char *tempX, *tempY, dataX[100], dataY[100], *ptr;
    tempX = strtok(data, ",");
    tempY = strtok(NULL, ",");
    strcpy(dataX, tempX);
    strcpy(dataY, tempY);
    clientX = strtol(dataX, &ptr, 10);
    clientY = strtol(dataY, &ptr, 10);
}

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
    clientInfo* client = (clientInfo*)arg;
    pthread_detach(pthread_self());
    char check[50];
    char username[256];
    char password[256];
    char recv_data[256];
    bzero(&recv_data, 12);

    while(1) {
        int data = read(client->clientfd, &recv_data, 2);
        if(data == 0) break;
        recv_data[data] = '\0';
        printf("Received from client in share-socket %d: %s\n", client->clientfd, recv_data);
        if(strlen(recv_data) == 0){
            break;
        }
        // Register handle
        if(strcmp(recv_data, "1") == 0){
            printf("Received from client in share-socket %d: Register\n", client->clientfd);
            data = read(client->clientfd, &username, 256);
            if(data == 0){
                exit(0);
            }
            username[data] = '\0';
            if (username[strlen(username) - 1] == '\n')
                username[strlen(username) - 1] = 0;
            node checkAcc = findExistAccount(head, username);
            if(checkAcc != NULL) {
                write(client->clientfd, "NotOK", 6);
            } else {
                write(client->clientfd, "OK", 3);
                data = read(client->clientfd, &password, 256);
                if(data == 0){
                    break;
                }
                password[data] = '\0';
                if (password[strlen(password) - 1] == '\n')
                    password[strlen(password) - 1] = 0;
                printf("Receive username from client in share-socket %d: %s\n", client->clientfd, username);
                printf("Receive password from client in share-socket %d: %s\n", client->clientfd, password);
                addTail(head, username, password, 1);
                printFile(head);
            }

        } else if(strcmp(recv_data, "2") == 0) {    // Login handle
            printf("Received from client in share-socket %d: Login\n", client->clientfd);
            
            data = read(client->clientfd, &username, 256);
            if(data == 0){
                exit(0);
            }
            username[data] = '\0';
            if (username[strlen(username) - 1] == '\n')
                username[strlen(username) - 1] = 0;
            node checkAcc = findExistAccount(head, username);
            if(checkAcc == NULL || checkAcc->status == 0) {
                write(client->clientfd, "NotOK", 6);
            } else {
                write(client->clientfd, "OK", 3);
                data = read(client->clientfd, &password, 256);
                if(data == 0){
                    break;
                }
                password[data] = '\0';
                if (password[strlen(password) - 1] == '\n')
                    password[strlen(password) - 1] = 0;
                printf("Receive username from client in share-socket %d: %s\n", client->clientfd, username);
                printf("Receive password from client in share-socket %d: %s\n", client->clientfd, password);
                while(strcmp(checkAcc->password, password) != 0){
                    write(client->clientfd, "Password not correct!", 21);
                    data = read(client->clientfd, &password, 256);
                    if(data == 0){
                        break;
                    }
                    password[data] = '\0';
                    if (password[strlen(password) - 1] == '\n')
                    password[strlen(password) - 1] = 0;
                }
                write(client->clientfd, "CorrectPass", 12);
                back:
                fflush(stdin);
                data = read(client->clientfd, &recv_data, 2);
                if(data == 0){
                    break;
                }
                recv_data[data] = '\0';
                printf("%s", recv_data);
                int t = 1;
                if(strcmp(recv_data, "1") == 0){    // Waiting room
                    switch(t) {
                        case 1: 
                            if(strcmp(room, "") == 0) {
                                strcpy(room, username);
                            }
                            printf("Host: %s", room);
                            write(client->clientfd, room, 256);
                            break;
                        default:
                            break;
                    }
                    while(1) {
                        printf("Loop");
                        data = read(client->clientfd, &check, 50);
                        if(data == 0) {
                            break;
                        }
                        check[data] = '\0';
                        switch(check[0]) {
                            case 's':
                            case 'S':
                                if(cli_count == 2) {
                                    write(client->clientfd, "Play", 5);
                                } else {
                                    write(client->clientfd, "Waiting", 8);
                                }
                                break;
                            case 'q':
                            case 'Q':
                                write(client->clientfd, "Quit", 5);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                } else if(strcmp(recv_data, "2") == 0){ // Change password
                    printf("Received from client in share-socket %d: Change password\n", client->clientfd);
                    char new[256];
                    data = read(client->clientfd, &new, 256);
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
                    printf("Received from client in share-socket %d: Quit game\n", client->clientfd);
                    break;
                } else {
                    client->ready = 1; // beegin to play
                }
            }
        }
    }
}
void *listen_client(void *args){
	        char recv_data[256];
            bzero(&recv_data, 12);
            if(pthread_detach(pthread_self())) {
		        printf("pthread_detach error\n");
	        }
            clientInfo* client = (clientInfo*)args;
            int data = read(client->clientfd, &recv_data, 2);
            if(data != 0) { 
                recv_data[data] = '\0';
                if(strlen(recv_data) > 0) {
                    
                    pthread_mutex_lock(&mutex);
                    decode_server(recv_data);
                    if( client->pos == LEFT ) {
                        left->center->x = clientX;
                        left->center->y = clientY;
                        char* send_data = encode_server(
                            left->center->x, 
                            left->center->y, 
                            ball->center->x, 
                            ball->center->y);
                        write(client->rivalfd, &send_data, strlen(send_data));
                    } else {
                        right->center->x = clientX;
                        right->center->y = clientY;
                        char* send_data = encode_server(
                            left->center->x, 
                            left->center->y, 
                            ball->center->x, 
                            ball->center->y);
                        write(client->rivalfd, &send_data, strlen(send_data));
                    }
                    
                    pthread_mutex_unlock(&mutex);
                }
                // mutex-lock ball and 2 paddle to update and send back

            }
	        pthread_exit(NULL);
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
    clientInfo client[2];
    while(1) {
        for(i = 1;; i++){
            socket_fds[i] = accept(socket_fds[0], (struct sockaddr *) &socket_addr[i], &len);
            if (socket_fds[i] < 0) 
                perror("ERROR on accept");
            if(i < 3) { // create thread to handle client request
                // i = 1 -> left paddle, i = 2 --> right paddle
                client[i-1].clientfd = socket_fds[i];
                client[i-1].rivalfd = socket_fds[3-i];
                pthread_t tid;
                int res = pthread_create(&tid, NULL, &client_handler, (void *)&client[i-1]);
                if( res != 0 ) {
                    perror("Thread created failure");
                    exit(EXIT_FAILURE);
                }
                if( client[i-1].ready == 1) {
                    if( i == 1) {
                        client->pos = LEFT;
                        write(socket_fds[i], "left", 4);
                    } else if( i == 2 ) {
                        client->pos = RIGHT;
                        write(socket_fds[i], "right", 5);
                    }
                }
                
            } else {    // if more than 2 clients have access => reject
                close(socket_fds[i]);
                continue;
            }
        }
        //close(socket_fds[0]);  
    }

game_setup:
    left = setPaddle(1, ROWS/2, 2); // left paddle
    right = setPaddle( COLS - 2, ROWS/2, 2); // right paddle
    ball = setBall( COLS/2, ROWS/2, 2, 2); // ball
    int randomX = 2 + rand() % (4 + 1 - 1); // X : 2, 3, 4, 5
    int randomY = 2;
    ball->plus_x = ball->plus_x > 0 ? randomX : -1*randomX;
    ball->plus_y = ball->plus_y > 0 ? randomY : -1*randomY;
    ball->center->x = COLS/2;
    ball->center->y = ROWS/2;
    while(1) { // loop to listen client keypress
        pthread_t tid1, tid2;
        int res = pthread_create(&tid1, NULL, &listen_client, (void *)&client[0]);
        if( res != 0 ) {
            perror("Thread created failure");
            exit(EXIT_FAILURE);
        }

        res = pthread_create(&tid2, NULL, &listen_client, (void *)&client[1]);
        if( res != 0 ) {
            perror("Thread created failure");
            exit(EXIT_FAILURE);
        }
        updatePosition(ball);
        // checking the conflict
        if( checkConfilctWithLeftPaddle(ball, left) || checkConfilctWithRightPaddle(ball, right)) {
            //conflict with paddle
            ball->plus_x = -1 * ball->plus_x;
        } else if( checkConflictWithWindow(ball, COLS, ROWS) ) {
            // conflict with window
            ball->plus_y = -1 * ball->plus_y;
        }

        // restart position
        if( ball->center->x > COLS - 2 || ball->center->x < 1 ) {
            goto game_setup;
        }
        // loop cycle - 0.5s
        usleep(500000);
    }
    return 0;
}