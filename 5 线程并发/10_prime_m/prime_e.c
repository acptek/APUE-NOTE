// prime.c的完善，在线程创建和线程退出时进行指针的传递

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT - LEFT + 1)

// 构造一个数据结构用于线程创建和退出时传值
struct thr_arg_st{
    int n;
};

static void *thr_prime(void *p){
    int i = ((struct thr_arg_st*)p)->n;
    // free(p);
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
    pthread_exit(p);
}

int main()
{

    int err;
    int i;
    pthread_t tid[THRNUM];
    struct thr_arg_st *p;
    void * ptr;
    for(i = LEFT; i <= RIGHT; ++i){
        p = malloc(sizeof(*p));
        if(p == NULL){
            perror("malloc");
            exit(1);
        }
        p->n = i;
        err = pthread_create(tid+i-LEFT, NULL, thr_prime, p);
        if(err){
            fprintf(stderr, "Create Error:%s\n",strerror(err));
            // for recycle
            exit(1);
        }
    }

    for(i = LEFT; i <= RIGHT; ++i){
        pthread_join(tid[i - LEFT], &ptr);
        free(ptr);
    }

    exit(0);
}
