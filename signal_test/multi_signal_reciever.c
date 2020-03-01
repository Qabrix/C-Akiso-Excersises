#include <signal.h>
#include <stdio.h>

void signal_handler(int dummy){
    printf("Złapano sygnał %d\n", dummy);
}

main(){
    for(int i=1; i<=64; i++)
        signal(i,signal_handler);

    while (1){
    }
    
}