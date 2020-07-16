#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM 4

static int num = 0;
static pthread_mutex_t mutex_num = PTHREAD_MUTEX_INITIALIZER;

static void *thr_prime(void *p){

    int i, j, mark;

    while (1) {
        pthread_mutex_lock(&mutex_num);
        while (num == 0) {
            pthread_mutex_unlock(&mutex_num);
            sched_yield();
            pthread_mutex_lock(&mutex_num);
        }
        if(num == -1) {
            pthread_mutex_unlock(&mutex_num);
            break;
        }
        i = num;
        num = 0;
        pthread_mutex_unlock(&mutex_num);

        mark = 1;
        for (j = 2; j * j < i; ++j) {
            if (i % j == 0) {
                mark = 0;
                break;
            }
        }
        if (mark)
            printf("[%d]  %d is a Prime\n", *(int *) p, i);
    }

    pthread_exit(p);
}

int main()
{

    int err, i;
    void *ptr;
    pthread_t tid[THRNUM];
    for(i = 0; i < THRNUM; ++i){
        int *wh = (int*)malloc(sizeof(int));
        *wh = i;
        err = pthread_create(tid+i, NULL, thr_prime, wh);
        if(err){
            fprintf(stderr, "Create Error:%s\n",strerror(err));
            // for recycle
            exit(1);
        }
    }

    for(i = LEFT; i <= RIGHT; ++i){
        pthread_mutex_lock(&mutex_num);
        while(num != 0){
            pthread_mutex_unlock(&mutex_num);
            sched_yield();
            pthread_mutex_lock(&mutex_num);
        }
        num = i;
        pthread_mutex_unlock(&mutex_num);
    }

    pthread_mutex_lock(&mutex_num);
    while(num != 0){
        pthread_mutex_unlock(&mutex_num);
        sched_yield();
        pthread_mutex_lock(&mutex_num);
    }
    num = -1;
    pthread_mutex_unlock(&mutex_num);

    for(i = 0; i < THRNUM; ++i){
        pthread_join(tid[i], &ptr);
        free(ptr);
    }

    exit(0);
}



/**
1 初始状态下，下游的n个线程被创建。在一个线程被创建后，它就向主线程发送一个信号，表示当前空闲
 主线程在收到下游线程的信号后，将对应线程的空闲状态置为1，主线程维护一个线程状态表

2 主线程遍历线程状态表来寻找空闲线程分配其任务
 线程信号到来时可能会同时到来多个，需要主线程中while接收，然后更改空闲状态

 上两步可归于一起：
    首先主线程中while接收信号，每当来一个信号：
        若当前有任务则分配其该任务，无任务则加新任务分配给一个到来的空闲线程，
        但是wait-pthread_join接收的是退出的线程，所以需要更换接收信号的方式
        -》即为主线程接收其他类型信号，但同样需要通过while不断接收
        -》
        解决方案：
        1 能否预先检测当前main线程中同一信号的pending位，若已经为1，则出让调度器一段时间，然后继续检测直到成功然后发送
            但是，即便上述成立，在我处理一个信号时要消耗一定的时间，那么这段时间内下一个信号会丢失吗
        2 设置多个信号位，来表示不同的线程发出的信号，此时main线程就不需要while循环接收，只需要将当前的处理结果发送任务返回。
            但是这个还是存在一些问题的，即便是不同的信号，但当同时到来时，仍然会出现 互斥量的使用，需要进行一定的处理，解决方案1中也是如此
 */
