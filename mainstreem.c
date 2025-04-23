#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "directory_struct.h"

//-------------- global values ---------------------------------
char* username = "oepickle";
char* hostname = "UNI-CTJ";
Node* nowNode;
char d_path[30];
char command[30];
char* cToken[10];
int cTokenLen = 0;
int pipeFlag = 1;

int playCd(char* str);
void playLs();
int checkCommand();
void pipelineCheck(char* token);
int programstart(char* cmd);
//------------------------------------------------------------------



int main() {
    directoryStart();
    strcpy(d_path, root->name);
    nowNode = root;

    while (1) {
        printf("%s@%s:%s$ ", username, hostname, d_path);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) break;

        if (strchr(command, ';')!=NULL){
            char* token = strtok(command, ";");
            while (token != NULL) {
                while (*token == ' ') token++;
                pipelineCheck(token);
                token = strtok(NULL, ";");
            }
        }
        else {
            pipelineCheck(command);
            //programstart(command);
        }
        
    }
}
//-----command start----------------
int checkCommand(char* cmd) {
    if (strncmp(cmd, "cd", 2) == 0) {
        return 1;
    }
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
    else {
        printf("%s: command not found\n", cmd);
        return 0;
    }
}

int programstart(char *cmd){
    int cdFlag = 0;
    while (*cmd == ' ') cmd++;

    if (strcmp(cmd, "cd") == 0) {
        cdFlag++;
    }
    else if (strncmp(cmd, "cd ", 3)==0) {
        char* cmd2 = cmd + 3;
        if (cmd2[0]=='/') {
            if (cmd2[1] == '\0') {
                cdFlag = 1;
                strcpy(cmd2, "/");
                nowNode = root;
            }
            else {
                char* context;
                char* token = strtok_r(cmd2+1, "/", &context);
                while (token != NULL) {
                    cdFlag = playCd(token);
                    if (cdFlag == 0) {
                        printf("Invalid directory name,\n");
                        return 0;
                    }
                    token = strtok_r(NULL, "/", &context);
                }
                return 1;
            }
        }
        else {
            cdFlag = playCd(cmd2);
            return cdFlag;
        }
    }
    else {
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
    return cdFlag;
}

//------------ pipeline check------------------------------
void pipelineCheck(char* token) {
    char temp[100];
    strcpy(temp, token);
    char* andOper = strstr(temp, "&&");
    char* orOper = strstr(temp, "||");
    if (andOper != NULL) {
        *andOper = '\0';
        char* left = temp;
        char* right = andOper + 2;
        while (*left == ' ') left++;
        while (*right == ' ') right++;
        if (programstart(left) != 0) {
            programstart(right);
        }
    }
    else if (orOper != NULL) {
        *orOper = '\0';
        char* left = temp;
        char* right = orOper + 2;
        while (*left == ' ') left++;
        while (*right == ' ') right++;
        if (programstart(left) == 0) {
            programstart(right);
        }
    }
    else {
        programstart(token);
    }
}

int playCd(char* token)
{
    int returnVal = 0;
    if (strlen(token) == 0 || token[0] == ' ') {
        printf("Invalid Syntax\n");
        return 0;
    }
    else if (strcmp(token, ".") == 0) {
        return 1;
    }
    else if (strcmp(token, "..") == 0) {
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
    else {
        Node* current = nowNode->child;
        while (current != NULL) {
            if (current->name != NULL && strcmp(token, current->name) == 0) {
                nowNode = current;
                if (strcmp(d_path, "/") != 0) {
                    strcat(d_path, "/");
                }
                strcat(d_path, current->name);

                returnVal = 1;
                break;
            }
            current = current->sibling;
        }
        if (returnVal != 1) {
            returnVal = 0;
            return 0;
        }
        return returnVal;
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