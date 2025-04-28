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
char printArr[300];
int iOperCheck = 0;
char returnArr[100];
char buffer[100];
int isPipeMode = 0;

int playCd(char* str);
char* playLs();
void runPipe(char* left, char* right);
char* checkCommand(char* command);
void pipelineCheck(char* token);
int programstart(char* cmd);
char* playCat(char* filename, char arr[100]);
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

//------------ pipeline check------------------------------
void pipelineCheck(char* token) {
    char temp[100];
    strcpy(temp, token);

    char* andOper = strstr(temp, "&&");
    char* orOper = strstr(temp, "||");
    char* pipeOper = strstr(temp, "|");

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
    else if (pipeOper != NULL) { 
        *pipeOper = '\0';
        char* left = temp;
        char* right = pipeOper + 1;
        while (*left == ' ') left++;
        while (*right == ' ') right++;
        runPipe(left, right); 
    }
    else {
        programstart(temp);
    }
}

void runPipe(char* left, char* right) {
    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();
    if (pid1 == 0) { // 첫 번째 자식
        close(fd[0]); // 읽기 닫기
        memset(buffer, 0, sizeof(buffer));
        isPipeMode = 1;
        checkCommand(left); // 왼쪽 명령 실행
        write(fd[1], buffer, strlen(buffer) + 1);
        close(fd[1]);
        _exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) { // 두 번째 자식
        close(fd[1]); // 쓰기 닫기
        char buffer_local[100];
        memset(buffer_local, 0, sizeof(buffer_local));
        read(fd[0], buffer_local, sizeof(buffer_local));
        
        char* token = strtok(buffer_local, " ");
        while (token != NULL) {
            checkCommand(token);
            token = strtok(NULL, " ");
        }
        close(fd[0]);
        _exit(0);
    }

    close(fd[0]);
    close(fd[1]);

    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
}

int programstart(char* cmd) {
    int cdFlag = 0;
    while (*cmd == ' ') cmd++;

    if (strncmp(cmd, "cd", 2) == 0 || strncmp(cmd, "pwd", 3) == 0 ||
        strncmp(cmd, "echo", 4) == 0 || strncmp(cmd, "exit", 4) == 0) {
        checkCommand(cmd);
        return 1;
    }
    else {
        pid_t pid = fork();
        if (pid == 0) {
            // 자식 프로세스
            checkCommand(cmd);
            _exit(0);
        }
        else {
            int status;
            wait(&status);
            return WEXITSTATUS(status);
        }
    }
}

char* checkCommand(char* cmd) {
    returnArr[0] = '\0';
    buffer[0] = '\0';

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
    else if (strncmp(cmd, "cat ", 4) == 0) {
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        if (token == NULL) {
            if (!isPipeMode) {
                char buf[1000];
                while (fgets(buf, sizeof(buf), stdin) != NULL) {
                    printf("%s", buf);
                }
            }else {
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
        if (isPipeMode)
            sprintf(buffer, "%s: command not found", cmd);
        else
            printf("%s: command not found\n", cmd);
        return buffer;
    }
}
int playCd(char* token)
{
    int returnVal = 0;
    if (strlen(token) == 0 || token[0] == ' ') {
        if (isPipeMode)
            sprintf(buffer, "Invalid Syntax");
        else
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
            if (isPipeMode)
                sprintf(buffer, "Invalid directory name");
            else
                printf("Invalid directory name\n");
            return 0;
        }
        return returnVal;
    }
}

//---make ls---------------------------
char* playLs() {
    returnArr[0] = '\0'; // 결과 문자열 초기화

    Node* current = nowNode->child;
    File* file = nowNode->file;

    if (current == NULL && file == NULL) {
        if (!isPipeMode) printf("nothing\n");
        return NULL;
    }

    while (current != NULL) {
        if (!isPipeMode) printf("%s ", current->name);
        strcat(returnArr, current->name);
        strcat(returnArr, " ");
        current = current->sibling;
    }
    while (file != NULL) {
        if (!isPipeMode) printf("%s ", file->name);
        strcat(returnArr, file->name);
        strcat(returnArr, " ");
        file = file->next;
    }
    if (!isPipeMode) printf("\n");

    return returnArr;
}

char* playCat(char* str, char arr[100]) {
    File* file = nowNode->file;
    int found = 0;

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
        if (!isPipeMode)
            printf("%s: No such file\n", str);
        else {
            strcat(arr, str);
            strcat(arr, ": No such file ");
        }
    }

    return arr;
}