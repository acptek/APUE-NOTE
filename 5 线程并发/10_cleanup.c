#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void cleanup(char *p){
    puts(p);
}

void * func(void *f){
    puts("Thread RUN");

    pthread_cleanup_push(cleanup, "1");
    pthread_cleanup_push(cleanup, "2");
    pthread_cleanup_push(cleanup, "3");

    puts("Over");

    pthread_exit(NULL);
    pthread_cleanup_pop(1);
    // 如果参数为0，则不执行
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(1);
}

int main()
{
    pthread_t tid;
    int err;
    puts("BEGIN");
    err = pthread_create(&tid, NULL, func, NULL);
    if(err){
        fprintf(stderr, "pthread_create():%s", strerror(err));
        exit(1);
    }
    pthread_join(tid, NULL);
    puts("END");
    exit(0);
}
