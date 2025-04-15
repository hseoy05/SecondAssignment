#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "directory_struct.h"

char* username = "oepickle";
char* hostname = "UNI-CTJ";
Node* nowNode;
char* d_path=NULL;
char command[30];
char* cToken[10];
int cTokenLen = 0;

void playCd(char* str);
void playLs();
void checkCommand();

int main() {
    directoryStart();
    d_path = root->name;
    nowNode = root;
    while (strcmp(command, "exit") != 0) { //Until: Enter exit
        printf("[%s@%s:%s]$", username, hostname, d_path);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        pid_t pid = fork();

        if (pid == 0) {
            checkCommand();
            _exit(0);
        }
        else {
            wait(NULL);
        }
    }
}

//-----command start----------------
void checkCommand() {
    if (strcmp(command, "pwd") == 0) { //Now directory show.
        printf("%s\n", d_path);
    }
    else if (strcmp(command, "exit") == 0) { //exit program
        printf("exit program\n");
    }
    else if (strncmp(command, "echo ", 5) == 0) { //echo (print string)
        char* context;
        strtok_r(command, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        while (token != NULL) {
            printf("%s ", token);
            token = strtok_r(NULL, " ", &context);
        }
        printf("\n");
    }
    else if (strcmp(command, "ls") == 0) { //ls (print files)
        playLs();
    }
    else if (strncmp(command, "cd", 2) == 0) {// move directory
        if (strncmp(command, "cd ", 3) == 0) {
            char* context;
            strtok_r(command, " ", &context);
            char* token = strtok_r(NULL, " ", &context);

            while (token != NULL) {
                playCd(token);
                token = strtok_r(NULL, " ", &context);
            }
        }
    }
    else {
        printf("%s: command not found\n", command);
    }
}


// make cd-----------------------------------------------------
void playCd(char* str) {
    if (str == NULL || strcmp(str, " ") == 0) {
        printf("Invalid command");
        return;
    }

    if (strcmp(str, ".") == 0) {
        return;
    }

    if (strcmp(str, "..") == 0) {
        if (nowNode->parent != NULL) {
            nowNode = nowNode->parent;
            d_path = nowNode->name;
        }
        else {
            printf("there is no parent directory\n");
        }
        return;
    }

    Node* current = nowNode->child;
    while (current != NULL) {
        str[strcspn(str, "\n")] = '\0';
        if (strcmp(str, current->name) == 0) {
            nowNode = current;
            d_path = current->name;
            printf("%s", d_path);
            return;
        }
        current = current->sibling;
    }
    printf("cd: %s: No such directory\n", str);
}

//---make ls---------------------------
void playLs() {
    Node* current = nowNode->child;
    File* file = nowNode->file;

    if (current == NULL && file == NULL) {
        printf("nothing\n");
        return;
    }

    while (current != NULL) {
        printf("%s  ", current->name);
        current = current->sibling;
    }
    while (file != NULL) {
        printf("%s  ", file->name);
        break;
        //file = file->next;
    }
    printf("\n");
}