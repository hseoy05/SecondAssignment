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
int background = 0;
int isPipeMode = 0;

int playCd(char* str);
void rtrim(char* str);
char* playLs();
void runPipe(char* left, char* right);
char* checkCommand(char* command);
int pipelineCheck(char* token);
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
int pipelineCheck(char* token) {
    char temp[100];
    strcpy(temp, token);

    background = 0;
    int len = strlen(token);

    if (len > 0 && token[len - 1] == '&') {
        background = 1;
        token[len - 1] = '\0';
        while (len > 1 && token[len - 2] == ' ') {
            token[len - 2] = '\0';
            len--;
        }
    }

    char* andOper = strstr(temp, "&&");
    char* orOper = strstr(temp, "||");
    char* pipeOper = strstr(temp, "|");

    if (andOper != NULL) {
        *andOper = '\0';
        char* left = temp;
        char* right = andOper + 2;
        while (*left == ' ') left++;
        while (*right == ' ') right++;
        if (pipelineCheck(left) != 0) {
            pipelineCheck(right);
        }
    }
    else if (orOper != NULL) {
        *orOper = '\0';
        char* left = temp;
        char* right = orOper + 2;
        rtrim(left);
        rtrim(right);
        if (pipelineCheck(left) == 0) {
            pipelineCheck(right);
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
        int val = programstart(temp);
        return val;
    }
}

void runPipe(char* left, char* right) {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        isPipeMode = 1;
        checkCommand(left);
        _exit(0);
    }
    else {
        close(fd[1]);
        wait(NULL);
        char buf[400];
        ssize_t n = read(fd[0], buf, sizeof(buf) - 1);
        close(fd[0]);
        buf[n] = '\0';
        buf[strcspn(buf, "\n")] = '\0';
        char str[1200];
        snprintf(str, sizeof(str), "%s %s", right, buf);

        isPipeMode = 0;
        checkCommand(str);
    }
}

int programstart(char* cmd) {

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

        if (isPipeMode==0&&token == NULL) {
            return NULL;
        }
        if (!token) {
            if (!isPipeMode) printf("cd: missing argument\n");
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
            char buf[200];
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
        return " ";
    }
}
int playCd(char* token)
{
    rtrim(token);
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
        printf("nothing\n");
        return NULL;
    }

    while (current != NULL) {
        printf("%s ", current->name);
        current = current->sibling;
    }
    File* file = nowNode->file;
    while (file != NULL) {
        printf("%s ", file->name);
        file = file->next;
    }
    printf("\n");

    fflush(stdout);
    return NULL;
}

char* playCat(char* str, char arr[100]) {
    rtrim(str);
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
        strcat(arr, str);
        strcat(arr, ": No such file ");
    }
}

void rtrim(char* str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
}