#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio_ext.h>
#include <ncurses.h>
#include "ball.h"
#include "paddle.h"
#include "game_action.h"
#include "process.h" 

#define LEFT 0
#define RIGHT 1
#define ROWS 45
#define COLS 110

// Handle Ctrl C
void ctr_c_handler() {
    printf("\nGoodbye Hust!\n");
    exit(0);
}
Ball* ball;
Paddle *left, *right;
int paddleX, paddleY, ballX, ballY; // vi tri cua vot doi thu & vi tri bong
char encoded[256];   // vi tri cua vot khi da encode
int pos; // LEFT player or RIGHT player

void decode_client(char data[256]) {
    char *tempX, *tempY, dataX[100], dataY[100], 
        *tempX2, *tempY2, dataX2[100], dataY2[100], 
        *tempX3, *tempY3, dataX3[100], dataY3[100], *ptr;
    tempX = strtok(data, ";");
    tempY = strtok(NULL, ";");
    strcpy(dataX, tempX);
    strcpy(dataY, tempY);

    tempX2 = strtok(dataX, ",");
    tempY2 = strtok(NULL, ",");
    strcpy(dataX2, tempX2);
    strcpy(dataY2, tempY2);
    paddleX = strtol(dataX2, &ptr, 10);
    paddleY = strtol(dataY2, &ptr, 10);

    tempX3 = strtok(dataY, ",");
    tempY3 = strtok(NULL, ",");
    strcpy(dataX3, tempX3);
    strcpy(dataY3, tempY3);
    ballX = strtol(dataX3, &ptr, 10);
    ballY = strtol(dataY3, &ptr, 10);
}


void encode_client(int c_x, int c_y) {       // c_x, c_y is position of paddles
    char client_x[100], client_y[100];
    sprintf(client_x, "%d", c_x);
    sprintf(client_y, "%d", c_y);
    strcpy(encoded, client_x);
    strcat(encoded, ",");
    strcat(encoded, client_y);
}


int connectServer(int sockfd) {
    char test[BUFF_SIZE];
    char choice[2];
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    char tmp2[BUFF_SIZE];
    int signup = 0;

    while(1) {
        if(signup == 1){
            printf("|             **Sign up successful!**             |\n");
        }
        else if(signup == -2){
            printf("       **Error! Username already exists!**       \n");
        }

        printf("1. Register\n");
        printf("2. Login\n");
        __fpurge(stdin);
        fgets(choice, sizeof(choice), stdin);
        int check = choice[0] - '0';
        write(sockfd, choice, 2);
        switch (check) {
            case 1:
                printf("               **Register**               \n");
                printf("Username: ");
                __fpurge(stdin);
                fgets(username, sizeof(username), stdin);
                write(sockfd, username, BUFF_SIZE);
                read(sockfd, &test, 10);
                if(strcmp(test, "NotOK") == 0) {
                    signup = -2;
                    break;
                } else {
                    printf("Password: ");
                    __fpurge(stdin);
                    fgets(password,sizeof(password), stdin);
                    while(strlen(password) < 6){
                        printf("Password length must be greater than or equal to 6 characters!\n");
                        printf("Password: ");
                        __fpurge(stdin);
                        fgets(password,sizeof(password), stdin);
                    }
                    write(sockfd, password, BUFF_SIZE);
                }
                signup = 1;
                break;
            case 2:
                printf("                **Login**               \n");
                printf("Username: ");
                __fpurge(stdin);
                fgets(username,sizeof(username), stdin);
                write(sockfd, username, BUFF_SIZE);
                read(sockfd, &test, 10);
                if(strcmp(test, "NotOK") == 0) {
                    printf("Not found!\n");
                    break;
                } else {
                    strcpy(tmp, username);
                    printf("Password: ");
                    __fpurge(stdin);
                    fgets(password,sizeof(password), stdin);
                    write(sockfd, password, BUFF_SIZE);
                    read(sockfd, &test, BUFF_SIZE);
                    while(strcmp(test, "CorrectPass") != 0){
                        printf("Error! Wrong password!\n");
                        printf("Password: ");
                        __fpurge(stdin);
                        fgets(password,sizeof(password), stdin);
                        write(sockfd, password, BUFF_SIZE);
                        read(sockfd, &test, BUFF_SIZE);
                    }
                }
                back:
                __fpurge(stdin);
                if(signup == -3) {
                    printf("       **Password changed successfully!**        \n");
                    printf("_________________________________________________\n");
                } else {
                    printf(" ____________Logged in successfully!______________ \n");
                }
                printf("1. Join waiting-room\n");
                printf("2. Change password\n");
                printf("3. Quit game\n");
                printf("********************\n");
                __fpurge(stdin);
                fgets(choice, sizeof(choice), stdin);
                while(strlen(choice) == 0 || choice[0] < '1' || choice[0] > '3'){
                    printf("Input from 1 to 3");
                    __fpurge(stdin);
                    fgets(choice,sizeof(choice), stdin);
                }
                int check2 = choice[0] - '0';
                char new_password[BUFF_SIZE], choose[BUFF_SIZE];
                switch(check2) {
                    case 1:
                        write(sockfd, choice, 2);
                        while(1) {
                            printf("Press [S] to start game\n");
                            printf("Press [Q] to quit game!\n");
                            __fpurge(stdin);
                            fgets(choose, sizeof(choose), stdin);
                            if((strcmp(choose, "Q") == 0) || (strcmp(choose, "q") == 0)) {
                                printf("Bye!");
                                exit(0);
                            }
                            write(sockfd, choose, 2);
                            int data = read(sockfd, &tmp2, BUFF_SIZE);
                            if(data == 0) {
                                break;
                            }
                            tmp2[data] = '\0';
                            if(strcmp(tmp2, "Play") == 0) {
                                return 1;
                            } else if(strcmp(tmp2, "Quit") == 0){
                                return 0;
                            } else {
                                printf("Please wait another player...\n");
                            }
                        }
                        break;
                    case 2:
                        write(sockfd, choice, 2);
                        printf("New password: ");
                        __fpurge(stdin);
                        fgets(new_password, sizeof(new_password), stdin);
                        while(strlen(new_password) < 6){
                            printf("Password length must be greater than or equal to 6 characters!\n");
                            printf("New password: ");
                            __fpurge(stdin);
                            fgets(new_password,sizeof(new_password), stdin);
                        }
                        write(sockfd, new_password, BUFF_SIZE);
                        signup = -3;
                        goto back;
                        break;
                    case 3:
                        signup = 0;
                        write(sockfd, choice, 2);
                        return 0;
                        write(sockfd, choice, 2);                      
                        goto back;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in servaddr;
    char *SERV_ADDR = argv[1];

    signal(SIGINT, ctr_c_handler);

    if(argc != 3) {
        printf("Wrong input format!\n");
        exit(1);
    }
    
    if(checkValidPort(argv[2]) == 0) {
        printf("Invalid port!\n");
        exit(1);
    }

    int SERV_PORT = atoi(argv[2]);

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error");
        exit(0);
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERV_ADDR);

    if(inet_pton(AF_INET, SERV_ADDR, &servaddr.sin_addr) <= 0) { 
		perror("Error");
		exit(0);
	} 

	if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
		perror("Error");
		exit(0);
	} 
    
    int checkConnect2Server = connectServer(sock);
    if(checkConnect2Server == 0) {
        return 0;
    }

    printf("Game will start after: 5 seconds\n");
    sleep(1);
    printf("Game will start after: 4 seconds\n");
    sleep(1);
    printf("Game will start after: 3 seconds\n");
    sleep(1);
    printf("Game will start after: 2 seconds\n");
    sleep(1);
    printf("Game will start after: 1 seconds\n");
    sleep(1);

    ///// set draw screen mode
    initscr(); // init draw mode
    cbreak(); // input mode
    noecho(); // no reply to screen
    WINDOW* customWin = newwin(45, 110, 0, 0);
    keypad(customWin, TRUE); // allow press special key like arrow key
    curs_set(0); // disappear cursor in screen
    ///////////////////////////////////

game_play:
    left = setPaddle(1, ROWS/2, 2); // left paddle
    right = setPaddle( COLS - 2, ROWS/2, 2); // right paddle
    ball = setBall( COLS/2, ROWS/2, 2, 2); // ball
    if(pos == LEFT) {
        paddleX = COLS-2;
        paddleY = ROWS/2;
    } else {
        paddleX = 1;
        paddleY = ROWS/2;
    }
    ballX = COLS/2;
    ballY = COLS/2;
    char income[BUFF_SIZE]; // data income from server : paddle + ball
    while(1) {
        wclear(customWin); // clear old screen to draw the new ones
        box(customWin, '|', '='); // draw game board
        if(pos == LEFT) {
            // draw left paddle
            for( int i = left->center->y - left->halfLength; i <= left->center->y + left->halfLength; i++) {
                mvwaddch(customWin, i, 1, 'H');
            }
            for( int i = 0; i < 4; i++) {
                int ch;
                if ((ch = wgetch(customWin)) != ERR) {
                    if( ch == KEY_UP ) {
                        displace(left, -2, ROWS);
                        // create thread --> enemy
                    } else if( ch == KEY_DOWN ) {
                        displace(left, 2, ROWS);
                    }
                encode_client(left->center->x, left->center->y);
                write(sock, encoded, sizeof(encoded));
                }
                usleep(50000); // sleep 50ms
            }
            
            int data = read(sock, income, sizeof(income));
            if( data == 0) {
                break;
            } else {
                income[data] = '\0';
                decode_client(income);
            }
            right->center->x = paddleX;
            right->center->y = paddleY;
            ball->center->x = ballX;
            ball->center->y = ballY;
            for( int i = right->center->y - right->halfLength; i <= right->center->y + right->halfLength; i++) {
                mvwaddch(customWin, i, COLS - 2, 'H');
            }
        } else {
            // draw right paddle
            for( int i = right->center->y - right->halfLength; i <= right->center->y + right->halfLength; i++) {
                mvwaddch(customWin, i, COLS - 2, 'H');
            }
            for( int i = 0; i <  4; i++ ) {
                int ch;
                if ((ch = wgetch(customWin)) != ERR) {
                    if( ch == KEY_UP ) {
                        displace(right, -2, ROWS);
                    } else if( ch == KEY_DOWN ) {
                        displace(right, 2, ROWS);
                    }
                    encode_client(right->center->x, right->center->y);
                    write(sock, encoded, sizeof(encoded));
                }
                usleep(50000);// sleep 50ms
            }
            int data = read(sock, income, sizeof(income));
            if( data == 0) {
                break;
            } else {
                income[data] = '\0';
                decode_client(income);
            }
            
            left->center->x = paddleX;
            left->center->y = paddleY;
            ball->center->x = ballX;
            ball->center->y = ballY;
            for( int i = left->center->y - left->halfLength; i <= left->center->y + left->halfLength; i++) {
                mvwaddch(customWin, i, 1, 'H');
            }
        }
        // draw ball
        mvwaddch(customWin, ball->center->y, ball->center->x, 'O');
        // update screen with new draw 
        wrefresh(customWin);
        usleep(200000);
    }
    return 0;
}