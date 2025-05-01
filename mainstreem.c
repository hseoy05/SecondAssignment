#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "directory_struct.h"

//--------------------------------------------------------------
//-------------- global values ---------------------------------
char* username = "oepickle";
char* hostname = "UNI-CTJ";
Node* nowNode;
File* nowFile;
char d_path[30];
char command[30];
int pipeFlag = 1;
char catArr[100];
int isBackgroundMode = 0;
int isPipeMode = 0;
//-------------- global function ------------------------------
int pipelineCheck(char* token);
void runPipe(char* left, char* right);
int programstart(char* cmd);
void checkCommand(char* command);

int playCd(char* str);
void playLs();
char* playCat(char* filename, char arr[100]);
void rtrim(char* str);
//--------------------------------------------------------------
//--------------------------------------------------------------




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
        }
        
    }
}

//------------ &&, ||, & check ------------------------------
int pipelineCheck(char* token) {
    char temp[100];
    strcpy(temp, token);
    //--------------back ground play--------------------
    isBackgroundMode = 0;
    int len = strlen(token);

    if (len > 0 && token[len - 1] == '&') {
        isBackgroundMode = 1;
        token[len - 1] = '\0';
        while (len > 1 && token[len - 2] == ' ') {
            token[len - 2] = '\0';
            len--;
        }
    }
    //-----------------------------------------------
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
        rtrim(left);
        rtrim(right);
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
//----------------------------------------------------------------

//------------ make child process using fork --------------------
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
    catArr[0] = '\0';

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
        if (strchr(token, '/') != NULL) {
            if (strcmp(token, "/") == 0) {
                nowNode = root;
                strcpy(d_path, "/");
                return;
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
        printf("%s\n", cmd + 5);
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
            return;
        }

        while (token != NULL) {
            playCat(token, catArr);
            token = strtok_r(NULL, " ", &context);
        }
        printf("%s\n", catArr);
        return;
    }
    else if (strcmp(cmd, "ls") == 0) {
        playLs();
        return;
    }
}
//--------------------------------------------------------

//--------------make cd ------------------------------------
int playCd(char* token)
{
    rtrim(token);
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
char* playCat(char* str, char arr[100]) {
    rtrim(str);
    nowFile = nowNode->file;
    int found = 0;

    while (nowFile != NULL) {
        if (strcmp(nowFile->name, str) == 0) {
            found = 1;
            strcat(arr, nowFile->text);
            strcat(arr, " ");
            break;
        }
        printf("\n");
        nowFile = nowFile->next;
    }

    if (!found) {
        strcat(arr, str);
        strcat(arr, ": No such file ");
    }
}
//------------------------------------------------------

//------------공백 제거------------------------------------
void rtrim(char* str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
}
//-----------------------------------------------------