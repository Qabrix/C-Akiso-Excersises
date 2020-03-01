#include <stdio.h>
#include <signal.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define maxCmdSize 4096
#define maxArgSize 128

pid_t currentPid = 0;

void printCurDir(){
    char curDir[100];
    getcwd(curDir, 100);
    printf("\033[1;32m");
    printf("%s", curDir);
    printf("\033[1;33m lsh");
    printf("\033[0m$ ");
}

void sigIntHandler(int dummy){
    printf("\n");
    if(currentPid == 0){
        printCurDir();
        fflush(stdout);
    }
}

int countElementsOfArray(char** arr){
    int size = 0;
    while(*(arr++))
        size++;
    
    return size;
}

void splitString(char* lineString, char** wordsArray, char* delimiter){
    int argNum = 0;
    char* sCopy;
    
    char *ptr = strtok(lineString, delimiter);

	while (ptr != NULL){
		wordsArray[argNum++] = ptr;
		ptr = strtok(NULL, delimiter);
	}
}

int prepareCmd(char* input, char *cmd[maxCmdSize][maxArgSize]){
    int cmdNum = 0;
    char *cmdLine[maxCmdSize]={0};
    splitString(input, cmdLine, "|");

    while(cmdLine[cmdNum]){
        splitString(cmdLine[cmdNum], cmd[cmdNum], " \n");
        cmdNum++;
    }   
    
    return cmdNum;
}

int getWaitOption(int cmdNum, char *cmd[maxCmdSize][maxArgSize]){
    int numOfLastArgs = countElementsOfArray(cmd[--cmdNum]);
    int sLen = strlen(cmd[cmdNum][numOfLastArgs-1]);
    if(cmd[cmdNum][numOfLastArgs-1][sLen-1] == '&'){
        if(sLen > 1)
            cmd[cmdNum][numOfLastArgs-1][sLen-1] = '\0';
        else
            cmd[cmdNum][numOfLastArgs-1] = 0;

        return WNOHANG;
    }

    return 0;
}

void cd(char* path){
    chdir(path);
}

void redirectFromFile(FILE* filefd){
    dup2(fileno(filefd), 0);
    fclose(filefd);
}

void redirectToFile(FILE* filefd){
    dup2(fileno(filefd), 1);
    fclose(filefd);
}

void redirectErrToFile(FILE* filefd){
    dup2(fileno(filefd), 2);
    fclose(filefd);
}

void redirectToFiles(char** rest, FILE* filefdIn, FILE* filefdOut, FILE* filefdErr){
    while(*rest){
        if(strcmp(*rest,"<") == 0){
            *rest = 0;
            filefdIn = fopen(*(rest+1),"r");
            redirectFromFile(filefdIn);
            rest+=2;
        }else if(strcmp(*rest,">") == 0){
            *rest = 0;
            filefdOut = fopen(*(rest+1),"w");
            redirectToFile(filefdOut);
            rest+=2;
        }else if(strcmp(*rest,"2>") == 0){
            *rest = 0;
            filefdErr = fopen(*(rest+1),"w");
            redirectErrToFile(filefdErr);
            rest+=2;
        }
        rest++;  
    }
}

void redirectForPipe(int* fdIn, int* fd, int i, int numOfCmds){
    dup2(*fdIn, 0); 
    if (i+1<numOfCmds)
            dup2(fd[1], 1);    
                
    close(fd[0]);     
}

void performPipeCmd(char *cmd[maxCmdSize][maxArgSize], int waitOption, int numOfCmds){
    if(strcmp(cmd[0][0],"cd") == 0){
        cd(cmd[0][1]);
        return;
    }

    waitpid(-1, NULL, WNOHANG);
    int fd[2];
    int status,    
        fdIn = 0;

    pid_t childpid;

    FILE* filefdIn;
    FILE* filefdOut;
    FILE* filefdErr;

    if(pipe(fd) == -1) {
        perror("Pipe failed");
        exit(1);
    }

    for(int i = 0; i < numOfCmds; i++){
        switch ((childpid = fork())){
        case -1:
            perror("fork");
            break;
        case 0:
            redirectForPipe(&fdIn, fd, i, numOfCmds);
            redirectToFiles(cmd[i], filefdIn, filefdOut, filefdErr);
            
            execvp(cmd[i][0], cmd[i]);
            
            printf("execvp failure\n");
            exit(EXIT_FAILURE);
            break;
        default: 
            currentPid = childpid;
            waitpid(childpid, &status, waitOption);

            if(waitOption == WNOHANG)
                 setpgid(childpid, childpid);
                    
            close(fd[1]);
            fdIn = fd[0]; 
            break;
        }      
    }
}

int checkIfShouldStop(char* input, char* result){
    if(result == NULL) { printf("\n"); }
    if(strcmp(input,"exit\n") == 0 || result == NULL) { return 1; }
    else { return 0; }
}

int main(void){
    signal(SIGINT,sigIntHandler);
    char input[maxCmdSize], 
        *cmd[maxCmdSize][maxArgSize], 
        *result;

    int cmdNum,
        waitOption;

    cmdNum = waitOption = 0;

    while(1){
        currentPid = 0;
        printCurDir();
        memset(cmd, 0, sizeof(cmd));

        result = fgets(input, maxCmdSize, stdin);

        if(checkIfShouldStop(input, result)) { 
            break; 
        }
        if(strcmp(input,"\n") == 0) { 
            continue; 
        }
            
        cmdNum = prepareCmd(input, cmd);
        waitOption = getWaitOption(cmdNum, cmd);
        performPipeCmd(cmd, waitOption, cmdNum);
    }

    return 0;
}
