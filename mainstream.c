#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "directory_struct.h"

//--------------------------------------------------------------
//-------------- global values ---------------------------------
#define VALUE_SIZE 256

char* username = "oepickle";
char* hostname = "UNI-CTJ";

Node* nowNode;
File* nowFile;

char d_path[VALUE_SIZE];
char userInput[VALUE_SIZE];
char catOutput[VALUE_SIZE];

int isPipeMode = 0;
int isBackgroundMode = 0;
//-------------- global function ------------------------------
int pipelineCheck(char* token);
void runPipe(char* left, char* right);
int programstart(char* cmd);
void checkCommand(char* command);

int playCd(char* str);
void playLs();
void playCat(char* filename, char arr[VALUE_SIZE]);
void trim(char* str);
//--------------------------------------------------------------
//--------------------------------------------------------------




int main() {
    directoryStart();
    strcpy(d_path, root->name);
    nowNode = root;

    while (1) {
        printf("%s@%s:%s$ ", username, hostname, d_path);
        fgets(userInput, sizeof(userInput), stdin);
        userInput[strcspn(userInput, "\n")] = '\0';

        if (strcmp(userInput, "exit") == 0) break;

        if (strchr(userInput, ';')!=NULL){
            char* token = strtok(userInput, ";");
            while (token != NULL) {
                while (*token == ' ') token++;
                pipelineCheck(token);
                token = strtok(NULL, ";");
            }
        }
        else {
            pipelineCheck(userInput);
        }
        
    }
}

//------------ &&, ||, & check ------------------------------
int pipelineCheck(char* token) {
    trim(token);
    if(strlen(token)==0) return 0;

    char temp[100];
    strncpy(temp, token, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    //--------------back ground play--------------------
    isBackgroundMode = 0;
    char* cmd = token;
    int len = strlen(cmd);

    if (len > 0 && cmd[len - 1] == '&') {
        isBackgroundMode = 1;
        cmd[len - 1] = '\0';
        len = strlen(cmd);
        while (len > 1 && cmd[len - 2] == ' ') {
            cmd[len - 2] = '\0';
            len--;
        }
        int val = pipelineCheck(cmd);
        return val;
    }
    //-----------------------------------------------
    char* andOper = strstr(temp, "&&");
    char* orOper = strstr(temp, "||");
    char* pipeOper = strstr(temp, "|");

    if (andOper != NULL) {
        *andOper = '\0';
        char* left = temp;
        char* right = andOper + 2;
        trim(left);
        trim(right);
        if (pipelineCheck(left) != 0) {
            pipelineCheck(right);
        }
    }
    else if (orOper != NULL) {
        *orOper = '\0';
        char* left = temp;
        char* right = orOper + 2;
        trim(left);
        trim(right);
        if (pipelineCheck(left) == 0) {
            pipelineCheck(right);
        }
    }
    else if (pipeOper != NULL) { 
        *pipeOper = '\0';
        char* left = temp;
        char* right = pipeOper + 1;
        trim(left);
        trim(right);
        runPipe(left, right); 
    }
    else {
        int val = programstart(temp);
        return val;
    }
}
//----------------------------------------------------

//----------------check | pipeline-----------------------
void runPipe(char* left, char* right) {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        isPipeMode = 1;
        trim(left);
        checkCommand(left);
        _exit(0);
    }
    else {
        close(fd[1]);
        wait(NULL);
        char buf[400];
        ssize_t n = read(fd[0], buf, sizeof(buf) - 1);
        close(fd[0]);
        if(n>0) buf[n]='\0';
        else buf[0]='\0';
        buf[strcspn(buf, "\n")] = '\0';
        char str[500];
        trim(buf);
        trim(right);
        snprintf(str, sizeof(str), "%s %s", right, buf);
        isPipeMode = 0;
        checkCommand(str);
    }
}
//----------------------------------------------------------------

//------------ make child process using fork --------------------
int programstart(char* cmd) {
    trim(cmd);

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
            if (!isBackgroundMode) {
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
//-----------------------------------------------------------------

//--------------- go pwd, cd, echo, cat, ls ------------------------
void checkCommand(char* cmd) {
    trim(cmd);

    if (strcmp(cmd, "pwd") == 0) {
        printf("%s\n", d_path);
    }
    else if (strncmp(cmd, "cd", 2) == 0) {
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        if (isPipeMode==0&&token == NULL) {
            return;
        }
        if (!token) {
            if (!isPipeMode) printf("cd: missing argument\n");
        }
        else if (strchr(token, '/') != NULL) {
            if (strcmp(token, "/") == 0) {
                nowNode = root;
                strcpy(d_path, "/");
                return;
            }
            Node* prevNode = nowNode;
            char prevPath[30];
            strcpy(prevPath, d_path);

            nowNode = root;
            strcpy(d_path, "/");

            char* path = token + 1; 
            char* subContext;
            char* part = strtok_r(path, "/", &subContext);

            int success = 1;
            while (part != NULL) {
                if (!playCd(part)) {
                    success = 0;
                    break;
                }
                part = strtok_r(NULL, "/", &subContext);
            }
            if (!success) {
                printf("Invalid directory path\n");
                nowNode = prevNode;
                strcpy(d_path, prevPath);
            }
        }
        else {
            playCd(token);
        }
    }
    else if (strncmp(cmd, "echo", 4) == 0) {
        printf("%s\n", cmd + 5);
    }
    else if (strncmp(cmd, "cat", 3) == 0) {
        char* context;
        strtok_r(cmd, " ", &context);
        char* token = strtok_r(NULL, " ", &context);

        if (token == NULL&&isPipeMode!=0) {
            char buf[200];
            ssize_t n;
            int totalRead=0;
            while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
                totalRead += n;
                if(totalRead>100) break;
                write(STDOUT_FILENO, buf, n);
            }
            return;
        }
        catOutput[0] = '\0';

        while (token != NULL) {
            playCat(token, catOutput);
            token = strtok_r(NULL, " ", &context);
        }

        int len = strlen(catOutput);
        if (len > 0 && catOutput[len - 1] == '\n') {
            catOutput[len - 1] = '\0';
        }

        printf("%s\n", catOutput);
        return;
    }
    else if (strcmp(cmd, "ls") == 0) {
        playLs();
        return;
    }
    else {
        printf("Error: '%s' is Invalid command.\n", cmd);
    }
}
//--------------------------------------------------------

//--------------make cd ------------------------------------
int playCd(char* token)
{
    trim(token);
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
            printf("Invalid directory name\n");
            return 0;
        }
        return returnVal;
    }
}
//------------------------------------------------------------

//---make ls--------------------------------------------------
void playLs() {
    Node* current = nowNode->child;
    
    if (current == NULL&&nowNode->file == NULL) {
        printf("No files\n");
        return;
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
    return;
}
//-----------------------------------------------

//---------make cat-------------------------------
void playCat(char* str, char arr[VALUE_SIZE]) {
    trim(str);
    nowFile = nowNode->file;
    int found = 0;

    while (nowFile != NULL) {
        if (strcmp(nowFile->name, str) == 0) {
            found = 1;
            strcat(arr, nowFile->text);
            strcat(arr, "\n");
            break;
        }
        nowFile = nowFile->next;
    }

    if (!found) {
        strcat(arr, str);
        strcat(arr, ": No such file ");
    }
}
//------------------------------------------------------

//------------���� ����------------------------------------
void trim(char* str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
    int index = 0;
    while (str[index] == ' ' || str[index] == '\t') {
        index++;
    }
    if (index > 0) {
        memmove(str, str + index, strlen(str + index) + 1);  // +1 to move null terminator
    }
}
//-----------------------------------------------------
