#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 256
#define BUFF_SIZE 256

struct Accounts {
    char account[100];
    char password[100];
    int status;
};

struct LinkedList{
    char *username;
    char *password;
    int status;
    int wrongPassword;
    struct LinkedList *next;
};

typedef struct LinkedList *node;

node head = NULL;

node CreateNode(char *username, char *password, int status){
    node temp; // declare a node
    temp = (node)malloc(sizeof(struct LinkedList)); // Cấp phát vùng nhớ dùng malloc()
    temp->next = NULL;// Cho next trỏ tới NULL
    temp->username = username;
    temp->password = password;
    temp->status = status; 
    temp->wrongPassword = 3;
    return temp;
}
 
node addTail(node head, char *username, char *password, int status){
    node temp,p;
    temp = CreateNode(username, password, status);
    if(head == NULL){
        head = temp;     
    }
    else{
        p  = head;
        while(p->next != NULL){
            p = p->next;
        }
        p->next = temp;
    }
    return head;
}

void readFile() {
    FILE *fp;
    char account[100],password[100];
    int status;
    int count = 0;
    fp = fopen("account.txt", "r");

    while(fscanf(fp, "%s %s %d", account, password, &status) != EOF){
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
}

void printFile(node head) {
    FILE *wf;
    wf = fopen("account.txt", "w");
    for(node p = head; p != NULL; p = p->next){
        fprintf(wf, "%s %s %d\n", p->username, p->password, p->status);
    }
    fclose(wf);
}

int checkNumber(char ch) {
    if(ch >= '0' && ch <= '9') {
        return 1;
    } else {
        return 0;
    }
}

int checkValidPort(char *port) {
    for(int i = 0; i < strlen(port); i++) {
        if(checkNumber(port[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

node findExistAccount(node head, char *acc) {
    for(node p = head; p != NULL; p = p->next){
        if(strcmp(p->username, acc) == 0) {
            return p;
        }
    }
    return NULL;
}

int findAccountStatus(node head, char *acc) {
    for(node p = head; p != NULL; p = p->next){
        if(strcmp(p->username, acc) == 0) {
            if(p->status == 1) {
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

void changeStatus(node head, char *acc) {
    for(node p = head; p != NULL; p = p->next){
        if(strcmp(p->username, acc) == 0) {
            p->status = 0;
        }
    }
}

void changePassword(node head, char *acc, char *pass) {
    for(node p = head; p != NULL; p = p->next){
        if(strcmp(p->username, acc) == 0) {
            p->password = pass;
        }
    }
}

void Traverse(node head) {
    for(node p = head; p != NULL; p = p->next){
        printf("%s\t%s\t%d\n", p->username, p->password, p->status);
    }
}

static char *itoa_simple_helper(char *dest, int i) {
  if (i <= -10) {
    dest = itoa_simple_helper(dest, i/10);
  }
  *dest++ = '0' - i%10;
  return dest;
}

char *itoa_simple(char *dest, int i) {
  char *s = dest;
  if (i < 0) {
    *s++ = '-';
  } else {
    i = -i;
  }
  *itoa_simple_helper(s, i) = '\0';
  return dest;
}