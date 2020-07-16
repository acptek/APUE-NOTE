#include "mysem.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT - LEFT + 1)
#define N 4

static mysem_t *sem;

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

    // sleep(5);

    mysem_add(sem, 1);
    pthread_exit(NULL);
}

int main()
{

    int err;
    int i;
    pthread_t tid[THRNUM];

    sem = mysem_init(N);
    if(sem == NULL){ // error
        fprintf(stderr, "mysem_init() fail\n");
        exit(1);
    }

    for(i = LEFT; i <= RIGHT; ++i){
        mysem_sub(sem, 1);
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

    mysem_destroy(sem);

    exit(0);
}

/*
 * 信号量和任务池的区别
 *   任务池是将任务分配到固定的线程，如果所有线程繁忙则任务分配阻塞
 *      通过维护一个全局num的值，来进行互斥与条件变量的使用
 *      任务池中，需要对每个线程维护其完成一个任务的开始与结束状态
 *      即维护需要多维护一个线程的可重用信息，而信号量则是用完一个则丢弃
 *      一旦空出则进行通知
 *
 *   信号量依旧是分配所有任务，但处理每个任务的线程都是新的线程，
 *      这就使得由线程个数不足繁忙等待重用的问题变为了
 *      线程个数已到达标定上限，当一个运行完空出个数上限 再加一个新线程处理
 *      这个操作类似于分配令牌桶，将token设为线程上限，所需的是线程个数
 *      当这里的单一信号量变为了一个信号量数组，
 *          表示有多个任务，每个任务的处理都规定了一定的线程上限
 *          这个操作与令牌桶实现流控非常类似
 */
