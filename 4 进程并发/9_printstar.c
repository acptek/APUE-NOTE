#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler(int sig){
    write(1, "!", 1);
}

int main()
{
    signal(SIGINT, handler);


    alarm(5);

    for(int i = 0; i < 10; ++i){
        write(1, "*", 1);
        sleep(1);
    }
    exit(1);
}

