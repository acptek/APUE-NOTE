// 开启(RIGHT-LEFT+1)个线程并发计算LEFT到RIGHT中的每一个值是否为素数

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT - LEFT + 1)

static void *thr_prime(void *p){
    int i = (int)p;
    int j, mark;
    mark = 1;
    for (j = 2; j*j < i; ++j) {
        if(i % j == 0){
            mark = 0;
            break;
        }
    }
    if(mark)
        printf("%d is a Prime\n", i);
    pthread_exit(NULL);
}

int main()
{

    int err;
    int i;
    pthread_t tid[THRNUM];
    for(i = LEFT; i <= RIGHT; ++i){
        err = pthread_create(tid+i-LEFT, NULL, thr_prime, (void*)i);
        if(err){
            fprintf(stderr, "Create Error:%s\n",strerror(err));
            // for recycle
            exit(1);
        }
    }

    for(i = LEFT; i <= RIGHT; ++i){
        pthread_join(tid[i - LEFT], NULL);
    }

    exit(0);
}
