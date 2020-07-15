#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define THNUM 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int curnum = 0;

void * fun(void *p){
    int cur = (int)p;
    while(1){
        pthread_mutex_lock(&mutex);
        while(curnum != cur){
            pthread_cond_wait(&cond, &mutex);
        }
        printf("%c", 'a'+cur);
        curnum = (curnum+1)%4;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv){

    pthread_t tid[THNUM];

    for(int i = 0; i < THNUM; ++i){
        pthread_create(&tid[i], NULL, fun, (void *)i);
    }

    alarm(5);

    for(int i = 0; i < THNUM; ++i){
        pthread_join(tid[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    exit(0);
}
