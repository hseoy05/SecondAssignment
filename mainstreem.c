#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "directory_struct.h"

char* username = "oepickle";
char* hostname = "UNI-CTJ";
Node* nowNode;
char d_path[30];
char command[30];
char* cToken[10];
int cTokenLen = 0;
int pipeFlag = 1;

void playCd(char* str);
void playLs();
int checkCommand();
void pipelineCheck(char* token);
int programstart(char* cmd);

int main() {
    directoryStart();
    strcpy(d_path, root->name);
    nowNode = root;

    while (1) {
        printf("%s@%s:%s$ ", username, hostname, d_path);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) break;

        char* token = strtok(command, ";");
        pipelineCheck(token);
    }
}

void pipelineCheck(char* token) {
    if (token == NULL) return;
    while (*token == ' ') token++;

    char* command = strtok(token, ";");
    while (command != NULL) {
        while (*command == ' ') command++;

        if (strstr(command, "&&")) {
            char* left = strtok(command, "&&");
            char* right = strtok(NULL, "&&");
            while (*left == ' ') token++;
            if (programstart(left) != 0) programstart(right);
        }
        else if (strstr(command, "||")) {
            char* left = strtok(command, "||");
            char* right = strtok(NULL, "||");

            if (left) while (*left == ' ') left++;
            if (right) while (*right == ' ') right++;

            if (programstart(left) == 0) programstart(right);
        }
        else {
            programstart(command);
        }

        command = strtok(NULL, ";");
    }
}

//--------------------------------------------------
int programstart(char *cmd){
    if (strncmp(cmd, "cd ", 3) == 0) {
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);
        int returnVal = 1;

        if (strcmp(token, ".") == 0) {
            return 1;
        }

        if (strcmp(token, "..") == 0) {
            char* lastSlash = strrchr(d_path, '/');

            if (lastSlash != NULL && lastSlash != d_path) {
                *lastSlash = '\0';
            }
            else {
                strcpy(d_path, "/");
            }
            nowNode = nowNode->parent;
            return 1;
        }

        while (token != NULL) {
            Node* current = nowNode->child;
            int found = 0;
            while (current != NULL) {
                if (strcmp(token, current->name) == 0) {
                    nowNode = current;
                    if (strcmp("/", current->parent->name) != 0) strcat(d_path, "/");
                    strcat(d_path, current->name);
                    found = 1;
                    break;
                }
                current = current->sibling;
            }
            if (!found) {
                printf("cd: %s: No such directory\n", token);
                returnVal = 0;
            }
            token = strtok_r(NULL, " ", &context);
        }
        return returnVal;
    }
    pid_t pid = fork();

    if (pid == 0) {
        int ret = checkCommand(cmd);
        _exit(ret);
    }
    else {
        int status;
        wait(&status);
        return WEXITSTATUS(status);
    }
}

//-----command start----------------
int checkCommand(char* cmd) {
    if (strcmp(cmd, "pwd") == 0) { //Now directory show.
        printf("%s\n", d_path);
        return 1;
    }
    else if (strcmp(cmd, "exit") == 0) { //exit program
        printf("exit program\n");
        return 1;
    }
    else if (strncmp(cmd, "echo ", 5) == 0) { //echo (print string)
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        while (token != NULL) {
            printf("%s ", token);
            token = strtok_r(NULL, " ", &context);
        }
        printf("\n");
        return 1;
    }
    else if (strcmp(cmd, "ls") == 0) { //ls (print files)
        playLs();
        return 1;
    }
    else{
        printf("%s: command not found\n", cmd);
        return 0;
    }
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