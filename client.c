#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <ncurses.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio_ext.h>
#include "process.h"
#include "point.h"
#include "ball.h"
#include "paddle.h"
#include "game_action.h"
#include <locale.h>

#define BUFF_SIZE   256
#define PORT        5500
#define HEIGHT      24
#define WIDTH       80
#define FRUIT       -111
#define WALL        -1111
#define WALL2       -1112
#define BORDER      -99
#define REFRESH     0.15
#define WINNER      -94
#define ONGOING     -34
#define INTERRUPTED -30
#define UP_KEY      'W'
#define DOWN_KEY    'S'
#define DEFAULT_KEY 'n'
#define LEFT_SIDE 1
#define RIGHT_SIDE 2
WINDOW* win;
char key = DEFAULT_KEY;
int game_result = ONGOING;
int side;

Paddle *left, *right;
Ball* ball;
//Output error message and exit cleanly
void error(const char* msg){
    perror(msg);
    exit(0);
}

//Stevens, chapter 12, page 428: Create detatched thread
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

void Snake(){
    system("clear");
    printf("     _______  __    _  _______  ___   _  _______   \n");
    printf("    |       ||  |  | ||   _   ||   | | ||       |  \n");
    printf("    |  _____||   |_| ||  |_|  ||   |_| ||    ___|  \n");
    printf("    | |_____ |       ||       ||      _||   |___   \n");
    printf("    |_____  ||  _    ||       ||     |_ |    ___|  \n");
    printf("     _____| || | |   ||   _   ||    _  ||   |___   \n");
    printf("    |_______||_|  |__||__| |__||___| |_||_______|  \n");
    printf("\n");
}

void ctrl_c_handler(){
    printf("\nQuit game!.\n");
    exit(0);
}

void Menu(){
    // Snake();
    printf(" _________________________________________________ \n");
    printf("|                 => [1]. Register                |\n");
    printf("|                 => [2]. Login                   |\n");
    printf("|_________________________________________________|\n");
    printf("Play now: ");
}

char *showRoom(char room[]){
    Snake();
    printf(" __________________Waiting-room___________________ \n");
    const char space[2] = "_";
    char *token;
    char tmp[BUFF_SIZE];
    int i = 1;
    char *main_player;
    token = strtok(room, space);
    main_player = token;
    strcpy(tmp, token);
    printf(">[No %d]. %s\n", i, tmp);
    i++;
    token = strtok(NULL, space);
    while(token != NULL){
        strcpy(tmp, token);
        printf(">[No %d]. %s\n", i, tmp);
        i++;
        token = strtok(NULL, space);
    }
    printf(" _________________________________________________ \n");
    printf(" Waiting-room will be updated every 5 seconds!\n");
    return main_player;
}

void showProfile(char user[]){
    Snake();
    printf(" _____________________Profile_____________________ \n");
    const char space[2] = "_";
    char *token;
    char tmp[BUFF_SIZE];
    float playedtimes;
    float wontimes;
    float pw;
    token = strtok(user, space);
    strcpy(tmp, token);
    printf(">>Account     : %s\n", tmp);
    token = strtok(NULL, space);
    strcpy(tmp, token);
    if(strlen(tmp) > 1){
        playedtimes = (tmp[0] - '0')*10 + (tmp[1] - '0');
    }
    else playedtimes = tmp[0] - '0';
    printf(">>Played-times: %s\n", tmp);
    token = strtok(NULL, space);
    strcpy(tmp, token);
    if(strlen(tmp) > 1){
        wontimes = (tmp[0] - '0')*10 + (tmp[1] - '0');
    }
    else wontimes = tmp[0] - '0';
    printf(">>Won-times   : %s\n", tmp);
    if(wontimes == 0) pw = 0;
    else pw = playedtimes/wontimes;
    printf(">>PW-rate     : %.2f\n", pw);
}

int sign_to_server(int sockfd){
    int choice_lb;
    char test[BUFF_SIZE];
    char choice[2];
    char usename[BUFF_SIZE];
    char password[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    char tmp2[BUFF_SIZE];
    int signup = 0;
    // List l;
    // InitList(&l);
    // User *p;
    while(1){

        Snake();
        if(signup == 1){
            printf("|             **Sign up successful!**             |\n");
        }
        else if(signup == -2){
            printf("|       **Error! Username already exists!**       |\n");
        }
        Menu();
        __fpurge(stdin);
        gets(choice);
        // test = choice - '0';
        // printf("%s\n", test);
        int check = choice[0] - '0';
        write(sockfd, choice, 2);
        switch (check){
            case 1:
                Snake();
                printf(" ________________Register account__________________\n");
                printf("Username: ");
                __fpurge(stdin);
                gets(usename);
                write(sockfd, usename, BUFF_SIZE);
                read(sockfd, &test, 10);
                if(strcmp(test, "NotOK") == 0){
                    // printf("Error! Username already exists!\n");
                    signup = -2;
                    break;
                }else{
                    printf("Password: ");
                    __fpurge(stdin);
                    gets(password);
                    while(strlen(password) < 6){
                        printf("Password length must be greater than or equal to 6 characters!\n");
                        printf("Password: ");
                        __fpurge(stdin);
                        gets(password);
                    }
                    write(sockfd, password, BUFF_SIZE);
                }
                signup = 1;
                break;
            case 2:
                Snake();
                printf(" _________________Login account___________________\n");
                printf("Username: ");
                __fpurge(stdin);
                gets(usename);
                write(sockfd, usename, BUFF_SIZE);
                read(sockfd, &test, 10);
                if(strcmp(test, "NotOK") == 0){
                    printf("Error! Wrong username!\n");
                    break;
                }else{
                    strcpy(tmp, usename);
                    printf("Password: ");
                    __fpurge(stdin);
                    gets(password);
                    write(sockfd, password, BUFF_SIZE);
                    read(sockfd, &test, BUFF_SIZE);
                    while(strcmp(test, "OKchoi") != 0){
                        printf("Error! Wrong password!\n");
                        printf("Password: ");
                        __fpurge(stdin);
                        gets(password);
                        write(sockfd, password, BUFF_SIZE);
                        read(sockfd, &test, BUFF_SIZE);
                    }
                    back:
                    __fpurge(stdin);
                    Snake();
                    if(signup == -3){
                        printf("|       **Password changed successfully!**        |\n");
                        printf("|_________________________________________________|\n");
                    }
                    else{
                        printf(" ____________Logged in successfully!______________ \n");
                    }
                    printf("|             => [1]. Join waiting-room           |\n");
                    printf("|             => [2]. Change password             |\n");
                    printf("|             => [3]. Show profile                |\n");
                    printf("|             => [4]. Show leaderboard            |\n");
                    printf("|             => [5]. Quit game                   |\n");
                    printf("|_________________________________________________|\n");
                    if(signup == -10){
                        printf("Please reconnect in a few minutes because server is overloading ...\n");
                        printf("            We apologize for this inconvenience!\n");
                    }
                    printf("===> ");
                    __fpurge(stdin);
                    gets(choice);
                    while(strlen(choice) == 0 || choice[0] < '1' || choice[0] > '5'){
                        printf("===> ");
                        __fpurge(stdin);
                        gets(choice);
                    }
                    // test = choice - '0';
                    // printf("%s\n", test);
                    int check2 = choice[0] - '0';
                    char new_password[BUFF_SIZE];
                    switch(check2){
                        case 1:
                            write(sockfd, choice, 2);
                            read(sockfd, &test, BUFF_SIZE);
                            if(strcmp(test, "maxplayers") == 0){
                                printf("Please reconnect in a few minutes because the server is overloading ...\nWe apologize for this inconvenience!\n");
                                sleep(5);
                                printf("Please send issues to email : manhminno@gmail.com\n");
                                sleep(4);
                                return 0;
                            }
                            else if(strcmp(test, "running") == 0){
                                signup = -10;
                                goto back;
                            }
                            while(1){
                                char *test2 = showRoom(test);
                                if(strcmp(usename, test2) == 0){
                                    // free(test2);
                                    side = LEFT_SIDE;
                                    printf("\n You are host of the room, let's start game!\n");
                                    printf("        Press [S] to start game\n");
                                    printf("        Press [Q] to quit game!\n");
                                    printf(" Press any key to wait for more players...\n");
                                    printf("=>");
                                    gets(test);
                                    write(sockfd, test, BUFF_SIZE);
                                }else{
                                    side = RIGHT_SIDE;
                                    printf("\n Game will be started by host: %s!\n", test2);
                                    printf(" Press [Ctr + C] to quit game!\n");
                                    printf(" Press any key to wait for more players...\n");
                                    strcpy(test, "accc");
                                    write(sockfd, test, BUFF_SIZE);
                                }
                                int xxx = read(sockfd, &test, BUFF_SIZE);
                                if(xxx == 0) return 1;
                                if(strcmp(test, "start") == 0) return 1;
                                else if(strcmp(test, "quit") == 0) return 0;
                            }
                        case 2:
                            write(sockfd, choice, 2);
                            printf("New password: ");
                            gets(new_password);
                            while(strlen(new_password) < 6){
                                printf("Password length must be greater than or equal to 6 characters!\n");
                                printf("New password: ");
                                gets(new_password);
                            }
                            // printf("%s\n", new_password);
                            write(sockfd, new_password, BUFF_SIZE);
                            signup = -3;
                            goto back;
                            break;
                        case 3:
                            signup = 0;
                            write(sockfd, choice, 2);
                            read(sockfd, &test, BUFF_SIZE);
                            // printf("%s\n", test);
                            showProfile(test);
                            printf("Press enter to continue...");
                            getchar();
                            write(sockfd, choice, 2);                      
                            goto back;
                        case 4:
                            signup = 0;
                            write(sockfd, choice, 2);
                            Snake();
                            printf(" ___________________Leaderboard___________________ \n");
                            printf("|    => [1]. Ranking by number of played-times    |\n");
                            printf("|    => [2]. Ranking by number of won-times       |\n");
                            printf("|_________________________________________________|\n");
                            printf("===> ");
                            scanf("%d", &choice_lb);
                            while(choice_lb < 1 || choice_lb >2){
                                Snake();
                                printf(" ___________________Leaderboard___________________ \n");
                                printf("|    => [1]. Ranking by number of played-times    |\n");
                                printf("|    => [2]. Ranking by number of won-times       |\n");
                                printf("|_________________________________________________|\n");
                                printf("===> ");
                                __fpurge(stdin);
                                scanf("%d", &choice_lb);
                            }
                            if(choice_lb == 1){
                                write(sockfd, "1", 2);
                                Snake();
                                printf(" ______________________Leaderboard______________________ \n");
                                printf("| Top |    Account    | Played-times |Won-times| PW-rate|\n");
                                for(int i = 0; i < 9; i++){
                                    read(sockfd, &test, BUFF_SIZE);
                                    char *token;
                                    char tmp[BUFF_SIZE];
                                    float playedtimes;
                                    float wontimes;
                                    float pw;
                                    const char space[2] = "_";
                                    if(i<3){
                                        if(i == 0) printf("***%d***\t", i+1);
                                        else if(i == 1) printf(" **%d**\t", i+1);
                                        else if(i == 2) printf("  *%d*\t", i+1);
                                    }
                                    else printf("  [%d]\t", i+1);
                                    token = strtok(test, space);
                                    strcpy(tmp, token);
                                    printf("%-22s", tmp);
                                    token = strtok(NULL, space);
                                    strcpy(tmp, token);
                                    if(strlen(tmp) > 1){
                                        playedtimes = (tmp[0] - '0')*10 + (tmp[1] - '0');
                                    }
                                    else playedtimes = tmp[0] - '0';
                                    printf("%-11s", tmp);
                                    token = strtok(NULL, space);
                                    strcpy(tmp, token);
                                    if(strlen(tmp) > 1){
                                        wontimes = (tmp[0] - '0')*10 + (tmp[1] - '0');
                                    }
                                    else wontimes = tmp[0] - '0';
                                    printf("%-9s", tmp);
                                    if(wontimes == 0) pw = 0;
                                    else pw = playedtimes/wontimes;
                                    printf("%.2f\n", pw);
                                    // printf("%s\n", test);
                                }
                                printf("Press enter to continue...");
                                __fpurge(stdin);
                                getchar();
                                write(sockfd, choice, 2);                      
                                goto back;
                            }
                            else if(choice_lb == 2){
                                write(sockfd, "2", 2);
                                Snake();
                                printf(" ______________________Leaderboard______________________ \n");
                                printf("| Top |    Account    |Won-times| Played-times | PW-rate|\n");
                                for(int i = 0; i < 9; i++){
                                    read(sockfd, &test, BUFF_SIZE);
                                    char *token;
                                    char tmp[BUFF_SIZE];
                                    float playedtimes;
                                    float wontimes;
                                    float pw;
                                    const char space[2] = "_";
                                    if(i<3){
                                        if(i == 0) printf("***%d***\t", i+1);
                                        else if(i == 1) printf(" **%d**\t", i+1);
                                        else if(i == 2) printf("  *%d*\t", i+1);
                                    }
                                    else printf("  [%d]\t", i+1);
                                    token = strtok(test, space);
                                    strcpy(tmp, token);
                                    printf("%-19s", tmp);
                                    token = strtok(NULL, space);
                                    strcpy(tmp, token);
                                    if(strlen(tmp) > 1){
                                        playedtimes = (tmp[0] - '0')*10 + (tmp[1] - '0');
                                    }
                                    else playedtimes = tmp[0] - '0';
                                    // printf("%-11s", tmp);
                                    strcpy(tmp2, tmp);
                                    token = strtok(NULL, space);
                                    strcpy(tmp, token);
                                    if(strlen(tmp) > 1){
                                        wontimes = (tmp[0] - '0')*10 + (tmp[1] - '0');
                                    }
                                    else wontimes = tmp[0] - '0';
                                    printf("%-13s", tmp);
                                    printf("%-10s", tmp2);
                                    if(wontimes == 0) pw = 0;
                                    else pw = playedtimes/wontimes;
                                    printf("%.2f\n", pw);
                                    // printf("%s\n", test);
                                }
                                printf("Press enter to continue...");
                                __fpurge(stdin);
                                getchar();
                                write(sockfd, choice, 2);                      
                                goto back;
                            }
                        case 5:
                            signup = 0;
                            write(sockfd, choice, 2);
                            return 0;
                            write(sockfd, choice, 2);                      
                            goto back;
                        default:
                            break;
                    }
                    
                }
                break;

            default:
                break;
        }
    }
    return 1;
}

void* write_to_server(void* arg){
    int sockfd = *(int *) arg;
    struct timespec ts;
    ts.tv_sec = REFRESH;
    ts.tv_nsec = ((int)(REFRESH * 1000) % 1000)  * 1000000;
    while(game_result == ONGOING){
        nanosleep(&ts, NULL);
        int n = write(sockfd, &key, 1);
        if(n < 0) 
            error("ERROR writing to socket.");
    }
    return 0;
}

void* update_screen(void* arg){    
    int  sockfd = *(int*) arg;
    char data[2];
    int  i, j, n;

    while(game_result == ONGOING){

        //Recieve updated map from server
        int income_key;
        income_key = read(sockfd, data, 1);
        if(income_key <= 0)
            perror("acnbabc");
        if( strcmp(data, (char*)UP_KEY) == 0 ) {
            if( side == LEFT_SIDE) 
                displace(right, 2, HEIGHT);
            else if( side == RIGHT_SIDE) 
                displace(left, 2, HEIGHT);
        } else if( strcmp(data, (char*)DOWN_KEY) == 0 ) {
            if( side == LEFT_SIDE) 
                displace(right, -2, HEIGHT);
            else if( side == RIGHT_SIDE) 
                displace(left, -2, HEIGHT);
        }

        updatePosition(ball);
        // checking the conflict
        if( checkConfilctWithLeftPaddle(ball, left) || checkConfilctWithRightPaddle(ball, right)) {
            //conflict with paddle
            ball->plus_x = -1 * ball->plus_x;
        } else if( checkConflictWithWindow(ball, WIDTH, HEIGHT) ) {
            // conflict with window
            ball->plus_y = -1 * ball->plus_y;
        }
        clear();
        box(win, 0, 0);
        refresh();
        wrefresh(win);
        for( int i = left->center->y - left->halfLength; i <= left->center->y + left->halfLength; i++) {
                mvwaddch(win, i, 1, 'H');
        }
        for( int i = right->center->y - right->halfLength; i <= right->center->y + right->halfLength; i++) {
                mvwaddch(win, i, 1, 'H');
        }
        mvwaddch(win, ball->center->y, ball->center->x, 'O');
        
        refresh();
    }
    return 0;
}

int main(int argc, char *argv[]){
    int                 sockfd;
    struct sockaddr_in  serv_addr;
    struct hostent*     server;
    char                key_buffer;

    if (argc < 2){
        fprintf(stderr,"Please type: %s [server ip] to launch the game.\n", argv[0]);
        exit(0);
    }    
    //Getting socket descriptor 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    //Resolving host name
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host.\n");
        exit(0);
    }
    signal(SIGINT, ctrl_c_handler);
    //Sets first n bytes of the area to zero    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        
    //Converting unsigned short integer from host byte order to network byte order. 
    serv_addr.sin_port = htons(PORT);
    
    //Attempt connection with server
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    int check_play = sign_to_server(sockfd);
    if(check_play == 0) return 0;

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


    // Init ball, paddle
    left = setPaddle(1, HEIGHT/2, 2); // left paddle
    right = setPaddle( WIDTH - 2, HEIGHT/2, 2); // right paddle
    ball = setBall( WIDTH/2, HEIGHT/2, 2, 2); // ball
    //Create Ncurses Window, with input, no echo and hidden cursor
    initscr();      
    cbreak();
    noecho();
    start_color();
    use_default_colors();    
    curs_set(0);

    //Set window to new ncurses window
    win = newwin(HEIGHT, WIDTH, 0, 0);
    //Snake colours
    init_pair(0, COLOR_WHITE, COLOR_BLUE);
    init_pair(1, COLOR_WHITE, COLOR_RED);
    init_pair(2, COLOR_WHITE, COLOR_GREEN);
    init_pair(3, COLOR_WHITE, COLOR_YELLOW);
    init_pair(4, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(5, COLOR_WHITE, COLOR_CYAN);
    init_pair(6, COLOR_BLACK, COLOR_YELLOW);
    init_pair(7, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(8, COLOR_BLACK, COLOR_CYAN);
    init_pair(9, COLOR_BLACK, COLOR_WHITE);

    mvprintw((HEIGHT-20)/2 + 1, (WIDTH-58)/2,"     _______  __    _  _______  ___   _  _______   \n");
    mvprintw((HEIGHT-20)/2 + 2, (WIDTH-58)/2,"    |       ||  |  | ||   _   ||   | | ||       |  \n");
    mvprintw((HEIGHT-20)/2 + 3, (WIDTH-58)/2,"    |  _____||   |_| ||  |_|  ||   |_| ||    ___|  \n");
    mvprintw((HEIGHT-20)/2 + 4, (WIDTH-58)/2,"    | |_____ |       ||       ||      _||   |___   \n");
    mvprintw((HEIGHT-20)/2 + 5, (WIDTH-58)/2,"    |_____  ||  _    ||       ||     |_ |    ___|  \n");
    mvprintw((HEIGHT-20)/2 + 6, (WIDTH-58)/2,"     _____| || | |   ||   _   ||    _  ||   |___   \n");
    mvprintw((HEIGHT-20)/2 + 7, (WIDTH-58)/2,"    |_______||_|  |__||__| |__||___| |_||_______|  \n");
    mvprintw((HEIGHT-20)/2 + 10, (WIDTH-58)/2," Instructions:"); 
    mvprintw((HEIGHT-20)/2 + 12, (WIDTH-58)/2," - Use the keys [W], [A], [S], [D] to move your snake.");
    mvprintw((HEIGHT-20)/2 + 13, (WIDTH-58)/2," - Eat fruit to grow in length.");
    mvprintw((HEIGHT-20)/2 + 14, (WIDTH-58)/2," - Do not run in to other snakes, the game border, the game wall"); 
    mvprintw((HEIGHT-20)/2 + 15, (WIDTH-58)/2,"   or yourself.");
    mvprintw((HEIGHT-20)/2 + 16, (WIDTH-58)/2," - The first snake to reach length 10 wins!");
    mvprintw((HEIGHT-20)/2 + 17, (WIDTH-58)/2," - Press '.' to quit at any time.");
    mvprintw((HEIGHT-20)/2 + 19, (WIDTH-58)/2,"Press any key to start . . ."); 
    getch();

    //Start writing inputs to the server every REFRESH seconds and updating the screen
    make_thread(update_screen, &sockfd);
    make_thread(write_to_server, &sockfd);

    while(game_result == ONGOING){
        
        //Get player input with time out
        bzero(&key_buffer, 1);
        timeout(REFRESH * 1000);
        key_buffer = getch();
        key_buffer = toupper(key_buffer);
        if(key_buffer == '.'){
            game_result = INTERRUPTED;
            break;
        } else if((key_buffer == UP_KEY) || (key_buffer == DOWN_KEY) ) {
            key = key_buffer;
            // update paddle
            if( side == LEFT_SIDE && key == UP_KEY) {
                displace(left, 2, HEIGHT);
            } else if( side == LEFT_SIDE && key == DOWN_KEY) {
                displace(left, -2, HEIGHT);
            } else if( side == RIGHT_SIDE && key == UP_KEY) {
                displace(right, 2, HEIGHT);
            } else if( side == RIGHT_SIDE && key == DOWN_KEY) {
                displace(right, -2, HEIGHT);
            }
        } else key = DEFAULT_KEY;
    }

    //Show the user who won
    WINDOW* announcement = newwin(7, 35, (HEIGHT - 7)/2, (WIDTH - 35)/2);
    box(announcement, 0, 0);
    if (game_result == WINNER){
        mvwaddstr(announcement, 2, (35-21)/2, "Game Over - You WIN!");
        mvwaddstr(announcement, 4, (35-21)/2, "Press any key to quit.");
        wbkgd(announcement,COLOR_PAIR(2));
    } else{
        mvwaddstr(announcement, 2, (35-21)/2, "Game Over - you lose!");
        if(game_result > 0)
            mvwprintw(announcement, 3, (35-13)/2, "Player %d won.", game_result);
        mvwaddstr(announcement, 4, (35-21)/2, "Press any key to quit.");
        wbkgd(announcement,COLOR_PAIR(1));
    }
    mvwin(announcement, (HEIGHT - 7)/2, (WIDTH - 35)/2);
    wnoutrefresh(announcement);
    wrefresh(announcement);
    sleep(2);
    wgetch(announcement);
    delwin(announcement);
    wclear(win);
    
    echo(); 
    curs_set(1);  
    endwin();
    //Close connection
    close(sockfd);
    return 0;
}
