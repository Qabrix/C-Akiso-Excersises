#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char** argv[]){
    int pid = 0,
        signalsToSendNum = 10;
    if(argc > 1)
        pid = atoi(argv[1]);

    for(int i=0; i<signalsToSendNum; i++){
        printf("Wysylanie sygnalu do procesu %d\n", pid);
        kill(pid,SIGUSR1);
    }        
}