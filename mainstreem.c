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
    int fd1[2], fd2[2];
    pipe(fd1);
    pipe(fd2);

    pid_t pid1 = fork();
    if (pid1 == 0) { 
        close(fd1[0]); // 읽는 쪽 닫기
        dup2(fd1[1], STDOUT_FILENO); 
        close(fd1[1]);

        close(fd2[0]);
        close(fd2[1]);

        isPipeMode = 1;
        checkCommand(left); // left 명령어 실행
        _exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) { 
        close(fd1[1]); // 쓰는 쪽 닫기
        dup2(fd1[0], STDIN_FILENO); // 표준입력을 파이프 읽기 쪽으로 연결
        close(fd1[0]);

        close(fd2[0]);
        dup2(fd2[1], STDOUT_FILENO);
        close(fd2[1]);

        isPipeMode = 1;
        checkCommand(right); // right 명령어 실행
        _exit(0);
    }

    close(fd1[0]);
    close(fd1[1]);
    close(fd2[1]);

    char buf[100];
    ssize_t n;
    while ((n = read(fd2[0], buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, n);
    }
    close(fd2[0]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int programstart(char* cmd) {
    int background = 0;
    int len = strlen(cmd);

    if (len > 0 && cmd[len - 1] == '&') {
        background = 1;
        cmd[len - 1] = '\0'; // &를 문자열에서 제거
        while (len > 1 && cmd[len - 2] == ' ') {
            cmd[len - 2] = '\0'; // 공백도 같이 제거
            len--;
        }
    }

    while (*cmd == ' ') cmd++;

    if (strncmp(cmd, "cd", 2) == 0 || strncmp(cmd, "pwd", 3) == 0 ||
        strncmp(cmd, "echo", 4) == 0 || strncmp(cmd, "exit", 4) == 0) {
        checkCommand(cmd);
        return 1;
    }
    else {
        pid_t pid = fork();
        if (pid == 0) {
            checkCommand(cmd);
            _exit(0);
        }
        else {
            if (!background) {
                int status;
                waitpid(pid, &status, 0);
                return WEXITSTATUS(status);
            }
            else {
                printf("[Background pid %d]\n", pid);
                return 0;
            }
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
        if (token == NULL && isPipeMode != 0) {
            char buf[100];
            ssize_t n;
            while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
                write(STDOUT_FILENO, buf, n);
            }
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
    else if (strncmp(cmd, "cat", 3) == 0) {
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        if (token == NULL&&isPipeMode!=0) {
            char buf[1024];
            ssize_t n;
            while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
                write(STDOUT_FILENO, buf, n);
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
    
    if (current == NULL&&nowNode->file == NULL) {
        if (!isPipeMode) {
            printf("nothing\n");
        }
        else {
            write(STDOUT_FILENO, "nothing\n", 8);
        }
        return NULL;
    }

    while (current != NULL) {
        if (!isPipeMode) {
            printf("%s ", current->name);
        }
        else {
            write(STDOUT_FILENO, current->name, strlen(current->name));
            write(STDOUT_FILENO, " ", 1); // 띄어쓰기
        }
        current = current->sibling;
    }
    while (nowNode->file != NULL) {
        if (!isPipeMode) {
            printf("%s ", nowNode->file->name);
        }
        else {
            write(STDOUT_FILENO, nowNode->file->name, strlen(nowNode->file->name));
            write(STDOUT_FILENO, " ", 1);
        }
        nowNode->file = nowNode->file->next;
    }
    if (!isPipeMode) {
        printf("\n");
    }
    else {
        write(STDOUT_FILENO, "\n", 1);
    }

    return NULL;
}

char* playCat(char* str, char arr[100]) {
    File* file = nowNode->file;
    int found = 0;

    while (file != NULL) {
        if (strcmp(file->name, str) == 0) {
            found = 1;
            if (isPipeMode) {
                write(STDOUT_FILENO, file->text, strlen(file->text));
                write(STDOUT_FILENO, " ", 1); // 띄어쓰기
            }
            else {
                strcat(arr, file->text);
                strcat(arr, " ");
            }
            break;
        }
        file = file->next;
    }

    if (!found) {
        if (isPipeMode) {
            char msg[100];
            snprintf(msg, sizeof(msg), "%s: No such file ", str);
            write(STDOUT_FILENO, msg, strlen(msg));
        }
        else {
            strcat(arr, str);
            strcat(arr, ": No such file ");
        }
    }

    return arr;
}