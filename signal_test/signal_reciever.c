#include <stdio.h>
#include <signal.h>

void signal_recived(int dummy){
    printf("Recieved signal %d\n", dummy);
}

int main(int argc, char** argv[]){
    signal(SIGUSR1, signal_recived);
    while(1){

    }
}