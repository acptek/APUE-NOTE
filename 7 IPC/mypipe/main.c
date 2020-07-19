#include <stdio.h>
#include <pthread.h>
#include "mypipe.h"


void * rd_fun(void * p){
    char buf[100];
    mypipe_register(p, 1);
    //puts("11122222222222222111");
    fflush(stdout);
    int k = mypipe_read(p, buf, 20);
    printf("Read %d Bytes From Pipe\n", k);
    for (int i = 0; i < k; ++i) {
        printf("%c", buf[i]);
    }printf("\n");

    mypipe_unregister(p, 1);
    pthread_exit(NULL);
}

void * wr_fun(void * p){
    mypipe_register(p, 2);
    //puts("11111111111111111111");
    fflush(stdout);
    printf("Write \"0123456789\" into Pipe\n");
    mypipe_write(p, "0123456789", 10);
    mypipe_unregister(p, 2);
    pthread_exit(NULL);
}

int main() {

    mypipe_t *pipe;
    pipe = mypipe_init();

    pthread_t tid1, tid2, tid3, tid4;
    pthread_create(&tid1, NULL, rd_fun, pipe);
    pthread_create(&tid2, NULL, wr_fun, pipe);
    pthread_create(&tid3, NULL, rd_fun, pipe);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_create(&tid4, NULL, wr_fun, pipe);
    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);
    mypipe_destroy(pipe);
    return 0;
}
