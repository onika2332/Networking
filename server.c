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
#include "process.h"
#include "ball.h"
#include "paddle.h"
#include "point.h"
#include "game_action.h"


#define DEFAULT_KEY 'n'
#define PORT                5500
#define MAX_PLAYERS         100
#define HEIGHT              24
#define WIDTH               80
#define MAX_SNAKE_LENGTH    HEIGHT * WIDTH
#define WINNER_LENGTH       10
#define WALL                -1111
#define WALL2               -1112
#define BORDER              -99
#define WINNER              -94
#define UP_KEY              'W'
#define DOWN_KEY            'S'
#define LEFT_KEY            'A'
#define RIGHT_KEY           'D'
 
int             someone_won = 0;
int check_run = 0;


int client[2];
char *room;
int start = 0;
int t1 = 5;
int t2 = 4;
char host[256];
int number_players = 0;

//Direction key types
typedef enum{
    UP    = UP_KEY, 
    DOWN  = DOWN_KEY, 
} direction;

int make_thread(void* (*fn)(void *), void* arg){
    int             err;
    pthread_t       tid;
    pthread_attr_t  attr;

    err = pthread_attr_init(&attr);
    if(err != 0)
        return err;
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(err == 0)
        err = pthread_create(&tid, &attr, fn, arg);
    pthread_attr_destroy(&attr);
    return err;
}

//Output error message and exit cleanly
void error(const char* msg){
    perror(msg);
    fflush(stdout);
    exit(1);
}

//Handle ctrl+c signal
void ctrl_c_handler(){
    printf("\nServer exited!.\n");
    exit(0);
}

void processRoom(char s[], char name[]){
    char tmp[256];
    const char space[2] = "_";
    char *token;
    token = strtok(s, space);
    char room_tmp[256];
    room_tmp[0] = '\0';
    strcat(room_tmp, "_");
    while(token != NULL){
        strcpy(tmp, token);
        if(strcmp(tmp, name) != 0){
            strcat(room_tmp, tmp);
        }
        token = strtok(NULL, space);
        if(token && strlen(room_tmp) != 1) strcat(room_tmp, "_");
    }
    if(room_tmp == NULL) s[0] = '\0';
    else strcpy(s, room_tmp);
}

//Thread gameplay function
void* gameplay(void* arg){ 
    if(number_players < 0) number_players = 0;
    if(strlen(room) == 0) host[0] = '\0';
    User *tmp;
    char usename[256];
    char password[256];
    List l;
    InitList(&l);
    readFile("nguoidung.txt", &l);
    int fd = *(int*) arg; 
    int player_no = fd-3;
    char recv_data[256];
    bzero(&recv_data, 12);
    int check_host = 0;
    while(1){
        int xxx = read(fd, &recv_data, 2);
        if(xxx == 0) break;
        recv_data[xxx] = '\0';
        printf("Received from client in share-socket %d: %s\n", fd, recv_data);
        if(strlen(recv_data) == 0){
            break;
        }
        if(strcmp(recv_data, "1") == 0){
            printf("Received from client in share-socket %d: Register\n", fd);
            xxx = read(fd, &usename, 256);
            if(xxx == 0){
                goto end;
            }
            usename[xxx] = '\0';
            tmp = checkUser(usename, l);
            if(tmp != NULL){
                write(fd, "NotOK", 6);
            }
            else{
                write(fd, "OK", 3);
                xxx = read(fd, &password, 256);
                if(xxx == 0){
                    goto end;
                }
                password[xxx] = '\0';
                printf("Received usename from client in share-socket %d: %s\n", fd, usename);
                printf("Received password from client in share-socket %d: %s\n", fd, password);
                int status = 0;
                int win_times = 0;
                User *p = makeUser(usename, password, status, win_times);
                addUser(&l, p);
                writeFile("nguoidung.txt", l);
            }
        }
        else if(strcmp(recv_data, "2") == 0){
            printf("Received from client in share-socket %d: Login\n", fd);
            xxx = read(fd, &usename, 256);
            if(xxx == 0){
                goto end;
            }
            usename[xxx] = '\0';
            tmp = checkUser(usename, l);
            if(tmp == NULL){
                write(fd, "NotOK", 6);
            }
            else{
                write(fd, "OK", 3);
                xxx = read(fd, &password, 256);
                if(xxx == 0){
                    break;
                }
                password[xxx] = '\0';
                printf("Receive usename from client in share-socket %d: %s\n", fd, usename);
                printf("Receive passwork from client in share-socket %d: %s\n", fd, password);
                while(strcmp(tmp->password, password) != 0){
                    write(fd, "Password sai!", 14);
                    xxx = read(fd, &password, 256);
                    if(xxx == 0){
                        break;
                    }
                    password[xxx] = '\0';
                }
                write(fd, "OKchoi", 7);
                // break;
                back:
                __fpurge(stdin);
                read(fd, &recv_data, 2);
                if(xxx == 0){
                    goto end;
                }
                recv_data[xxx] = '\0';
                if(strcmp(recv_data, "1") == 0){
                    printf("Received from client in share-socket %d: Join waitting-room\n", fd);
                    if(strlen(usename) < 1) goto end;
                    strcat(room, "_");
                    strcat(room, usename);
                    if(host[0] == '\0') strcpy(host, usename);
                    if(number_players == 9){
                        write(fd, "maxplayers", 11);
                        goto end;
                    }
                    if(check_run == 1){
                        write(fd, "running", 8);
                        room[0] = '\0';
                        goto back;
                    }
                    number_players += 1;
                    write(fd, room, 256);
                    printf("||Number of players are accessing: %d||\n", number_players);
                    while(1){
                        xxx = read(fd, &recv_data, 256);
                        if(xxx == 0){
                            // goto end;
                            break;
                        }
                        if(strcmp(recv_data, "S") == 0 || strcmp(recv_data, "s") == 0){
                            strcpy(recv_data, "start");
                            write(fd, recv_data, 256);
                            check_run = 1;
                            start = 1;
                            sleep(t2);
                            check_host = 1;
                            break;
                        }
                        else if(strcmp(recv_data, "Q") == 0 || strcmp(recv_data, "q") == 0){
                            strcpy(recv_data, "quit");
                            write(fd, recv_data, 256);
                            processRoom(room, tmp->usename);
                            if(strlen(room) == 1) room = '\0';
                            // number_players -= 1;
                            // printf("ssssss\n");
                            goto end;
                            break;
                        }
                        if(start == 1){
                            t1 = 3;
                            strcpy(recv_data, "start");
                            write(fd, recv_data, 256);
                            break;
                        }
                        else{
                            write(fd, room, 256);
                        }
                        // start = 1;
                        sleep(t1);
                    }
                    break;
                }
                else if(strcmp(recv_data, "2") == 0){
                    printf("Received from client in share-socket %d: Change password\n", fd);
                    char new[256];
                    xxx = read(fd, &new, 256);
                    if(xxx == 0) break;
                    // printf("nhan: %s\n", new);
                    strcpy(tmp->password, new);
                    writeFile("nguoidung.txt", l);
                    goto back;
                }
                else if(strcmp(recv_data, "5") == 0){
                    printf("Received from client in share-socket %d: Quit game\n", fd);
                    if(strcmp(recv_data, "5") == 0){
                        goto end;
                    }
                }
            }
        }
        // break;
    }
    printf("Player %d had connected to game!\n", player_no);
    if(tmp != NULL){
        tmp->status += 1;
        writeFile("nguoidung.txt", l);
    }
    // printf("%s\n", host);
    if(start == 1){
        room[0] = '\0';
        number_players = 0;
    }
    if(check_host == 0 && strcmp(tmp->usename, host) == 0){
        // start = 1;
        // room[0] = '\0';
        host[0] = '\0';
        // check_host = 0;
        processRoom(room, tmp->usename);
        // printf("%s---\n", room);
        if(strlen(room) == 1) room[0] = '\0';
        number_players -= 1;
    }
    else{
        processRoom(room, tmp->usename);
        start = 0;
        check_host = 0;
    }

    //Determine player number from file descriptor argument
    end:
    if(number_players != 0) number_players -= 1;
    printf("||Number of players are accessing: %d||\n", number_players);

    //Variables for user input
    char key = DEFAULT_KEY;
    char key_buffer;
    int  n;
    int  success = 1;

    while(success){
        //Player key input
        bzero(&key_buffer, 1);
        n = read(fd, &key_buffer, 1);
        if (n <= 0)
            break;

        //If user key is a direction, then apply it
        key_buffer = toupper(key_buffer);   
        if(  key_buffer == UP || key_buffer == DOWN )  
            key = key_buffer;

        switch(key){

            case UP:
            case DOWN:{
                for(int i=0; i<2; i++){
			            if(client[i] != fd){
                            if(write(client[i], (char*)key, sizeof(key)) < 0){
                                perror("ERROR: write to descriptor failed");
                                break;
                            }
                        }
                }
            }

            default: break;
        }   
    }
}

//Main function
int main(){
    room = (char *)malloc(256*sizeof(char));
    int                socket_fds[MAX_PLAYERS];     
    struct sockaddr_in socket_addr[MAX_PLAYERS];
    int                i;

    //Handle Ctrl+C
    signal(SIGINT, ctrl_c_handler);
    //Create server socket
    socket_fds[0] = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fds[0] < 0) 
        error("ERROR opening socket");
        
    //Set socket address to zero and set attributes
    bzero((char *) &socket_addr[0], sizeof(socket_addr[0]));  
    socket_addr[0].sin_family = AF_INET;
    socket_addr[0].sin_addr.s_addr = INADDR_ANY;
    //Converting unsigned short integer from host byte order to network byte order. 
    socket_addr[0].sin_port = htons(PORT);
    
    //Assigning address specified by addr to the socket referred by the server socket fd
    if (bind(socket_fds[0], (struct sockaddr *) &socket_addr[0], sizeof(socket_addr[0])) < 0) 
            error("ERROR on binding");

    //Marking socket as a socket that will be used to accept incoming connection requests  
    listen(socket_fds[0], 9);
    socklen_t clilen = sizeof(socket_addr[0]);

    while(1){
        for(i = 1; ; i++){
            //Accepting an incoming connection request
            socket_fds[i] = accept(socket_fds[0], (struct sockaddr *) &socket_addr[i], &clilen);
            if (socket_fds[i] < 0) 
                error("ERROR on accept");
            
            client[i-1] = socket_fds[i];
            if(someone_won){
                someone_won = 0;
            }
            make_thread(&gameplay, &socket_fds[i]); 
        }
        //Closing the server socket
        close(socket_fds[0]);  
    }
    return 0; 
}
