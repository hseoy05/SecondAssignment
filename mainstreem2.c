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
char command[30]; //input value
char* cToken[10];
int cTokenLen = 0;
int pipeFlag = 1;
char printArr[300];
int iOperCheck = 0;
char returnArr[100];
char buffer[100];
int isPipeMode = 0;

int fget();
char* parent(char *cmd);
char* child(char* cmd);

int playCd(char* str);
void forkStart(char* cmd);
char* playLs();
void runPipe(char* left, char* right);
char* checkCommand(char* command);
int pipelineCheck(char* token);
int programstart(char* cmd);
char* playCat(char* filename, char arr[100]);
//------------------------------------------------------------------


int main() {
	//make directory
	directoryStart();
	strcpy(d_path, root->name);
	nowNode = root;

	// get input value
    int exitVal = 0;
    while (exitVal == 0) {
        exitVal = fget();

        char* token = strtok(command, ";");
        do {
            while (*token == ' ') token++;
            programstart(token);
            token = strtok(NULL, ";");
        } while (token != NULL);
    }
}

//fget()--------------------------------------
int fget() {
    printf("%s@%s:%s$ ", username, hostname, d_path);
    if (fgets(command, sizeof(command), stdin) == NULL) {
        printf("\nexit\n");
        return 1;
    }
    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "exit") == 0) return 1;
}
//---------------------------------------------------


int programstart(char* cmd) {
    int pipeVal = pipelineCheck(cmd);

    char temp[100];
    strcpy(temp, cmd);

    if (pipeVal==-1) {
        forkStart(cmd);
    }
    else if (pipeVal == 1) {
        char* andOper = strstr(temp, "&&"); //1
        char* left = temp;
        char* right = andOper + 2;
        while (*left == ' ') left++;
        while (*right == ' ') right++;
        if (programstart(left) != 0) {
            programstart(right);
        }
    }
    else if (pipeVal == 2) {
        char* orOper = strstr(temp, "||"); //2
        char* left = temp;
        char* right = orOper + 2;
        while (*left == ' ') left++;
        while (*right == ' ') right++;
        if (programstart(left) == 0) {
            programstart(right);
        }
    }
    else {
        char* pipeOper = strstr(temp, "|"); //3
        *pipeOper = '\0';
        char* left = temp;
        char* right = pipeOper + 1;
        while (*left == ' ') left++;
        while (*right == ' ') right++;
        //runPipe(left, right);
    }
}

void forkStart(char* cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        child(cmd);
    }
    else if (pid > 0) {
        parent(cmd);
    }
    else {
        printf("Fail");
    }
}

//------------ pipeline check------------------------------
int pipelineCheck(char* token) {
    int answer = 0;
    char temp[100];
    strcpy(temp, token);

    char* andOper = strstr(temp, "&&"); //1
    char* orOper = strstr(temp, "||"); //2
    char* pipeOper = strstr(temp, "|"); //3

    if (andOper != NULL) {
        answer = 1;
    }
    else if (orOper != NULL) {
        answer = 2;
    }
    else if (pipeOper != NULL) {
        answer = 3;
    }
    else {
        answer = -1;
    }
    return answer;
}

char* parent(char* cmd) {
    //cd, pwd, echo
    returnArr[0] = '\0';
    buffer[0] = '\0';
    if (strncmp(cmd, "ls", 2) == 0 || strncmp(cmd, "cat", 3) == 0) {
        return NULL;
    }
    if (strcmp(cmd, "pwd") == 0) {
        if (isPipeMode)
            sprintf(buffer, "%s", d_path);
        else
            printf("%s\n", d_path);
        return buffer;
    }
    else if (strncmp(cmd, "cd", 2) == 0) {
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        if (token == NULL) {
            return NULL;
        }

        if (strchr(token, '/') != NULL) {
            if (strcmp(token, "/") == 0) {
                nowNode = root;
                strcpy(d_path, "/");
                return NULL;
            }
            char* path = token + 1;
            char* subContext;
            char* part = strtok_r(path, "/", &subContext);
            nowNode = root;
            strcpy(d_path, "/");
            while (part != NULL) {
                playCd(part);
                part = strtok_r(NULL, "/", &subContext);
            }
        }
        else {
            playCd(token);
        }
        return NULL;
    }
    else if (strncmp(cmd, "echo", 4) == 0) {
        if (isPipeMode)
            sprintf(buffer, "%s", cmd + 5);
        else
            printf("%s\n", cmd + 5);
        return buffer;
    }
}

char* child(char* cmd) {
    if (strncmp(cmd, "pwd", 3) == 0 || strncmp(cmd, "echo", 4) == 0 || strncmp(cmd, "cd", 2) == 0) {
        return NULL;
    }
    if (strncmp(cmd, "cat ", 4) == 0) {
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        if (token == NULL) {
            if (!isPipeMode) {
                char buf[1000];
                while (fgets(buf, sizeof(buf), stdin) != NULL) {
                    printf("%s", buf);
                }
            }
            else {
                buffer[0] = '\0';
            }
            return NULL;
        }

        while (token != NULL) {
            playCat(token, returnArr);
            token = strtok_r(NULL, " ", &context);
        }
        if (isPipeMode) {
            sprintf(buffer, "%s", returnArr);
        }
        else {
            printf("%s\n", returnArr);
        }
        return buffer;
    }
    else if (strcmp(cmd, "ls") == 0) {
        playLs();
        return buffer;
    }
    else {
        printf("%s: command not found\n", cmd);
    }
}


int playCd(char* token)
{
    int returnVal = 0;
    if (strlen(token) == 0 || token[0] == ' ') {
        printf("Invalid Syntax\n");
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
            printf("Invalid directory name\n");
        }
        return returnVal;
    }
}

//---make ls---------------------------
char* playLs() {
    returnArr[0] = '\0'; // 결과 문자열 초기화

    Node* current = nowNode->child;
    if (current == NULL) {
        printf("nothing\n");
    }

    while (current != NULL) {
        printf("%s ", current->name);
        current = current->sibling;
    }
    printf("\n");
    return NULL;
}

char* playCat(char* str, char arr[100]) {
    File* file = nowNode->file;
    int found = 0;
    arr[0] = '\0';

    while (file != NULL) {
        if (strcmp(file->name, str) == 0) {
            found = 1;
            strcat(arr, file->text);
            strcat(arr, " ");
            break;
        }
        file = file->next;
    }

    if (!found) {
        strcat(arr, str);
        strcat(arr, ": No such file ");
    }
}