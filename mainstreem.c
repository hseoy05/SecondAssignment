#include<stdio.h>
#include<string.h>
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
        checkCommand();
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
    else if (strcmp(command, "") == 0) {
        printf("");
    }
    else if (strcmp(command, "ls") == 0) {
        playLs();
    }
    else if (strncmp(command, "cd", 2) == 0) {
        char* context;
        strtok_s(command, " ", &context);
        char* token = strtok_s(NULL, ",", &context);
        
        while (token != NULL) {
            playCd(token);
            token = strtok_s(NULL, ",", &context);
        }
    }
    else {
        printf("%s: command not found\n", command);
    }
}


// make cd-----------------------------------------------------
void playCd(char* str) {
    if (str == NULL || strcmp(str,".")==0) return;

    if (strcmp(str, "..") == 0) {
        if (nowNode->parent != NULL) {
            nowNode = nowNode->parent;
            d_path = nowNode->name;
        }
        else {
            printf("there is no parent directory");
        }
        return;
    }

    Node* current = nowNode->child;
    while (current != NULL) {
        if (strcmp(str, current->name) == 0) {
            nowNode = current;
            d_path = current->name;
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
        printf("nothing");
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