#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define THNUM   4

static pthread_mutex_t mut[THNUM];

void * fun(void *p){
    int cur = (int *)p;
    while(1){
        printf("%d\n", cur);
        pthread_mutex_lock(&mut[cur]);
        printf("%c", 'a'+cur);
        pthread_mutex_unlock(&mut[(cur+1)%THNUM]);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv){

    pthread_t tid[THNUM];

    for(int i = 0; i < THNUM; ++i){
        pthread_mutex_init(&mut[i], NULL);
        pthread_mutex_lock(&mut[i]);
    }

    for(int i = 0; i < THNUM; ++i){
        pthread_create(&tid[i], NULL, fun, (void*)(i));
    }

    pthread_mutex_unlock(&mut[0]);

    alarm(5);

    for(int i = 0; i < THNUM; ++i){
        pthread_join(tid[i], NULL);
    }

    for(int i = 0; i < THNUM; ++i){
        pthread_mutex_destroy(&mut[i]);
    }
    exit(0);
}
