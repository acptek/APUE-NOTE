// 构建池类算法
// 当线程数量少时进行任务分配的方式

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
    
    // 线程循环做素数判断的工作，最初还没有分配任务：num = 0
    while (1) {
        pthread_mutex_lock(&mutex_num);
        while (num == 0) {
            pthread_mutex_unlock(&mutex_num);
            sched_yield();
            pthread_mutex_lock(&mutex_num);
        }
        
        // 表示所有任务已经完成
        if(num == -1) {
            pthread_mutex_unlock(&mutex_num);
            break;
        }
        
        // 在这里可能会出现一个问题，当有一个num修改为0失败，那么当前num值会进入到自己或者某个空闲线程在执行一次
        
        // num > 0 : 表示要判断素数的那个数
        i = num; // 
        num = 0; // 将num重置为0表示这个线程空闲
        pthread_mutex_unlock(&mutex_num); // 然后立即解锁

		// 接下来操作不会出现线程冲突的情况
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
    // 先创建THRNUM个线程，等待接收任务
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

	// 从这200个任务中挨个取任务，取到分配任务区域
	// 如果num不等于0表示当前有任务正在分派
	// 如果num等于0表示等待分派的任务已被分配了出去
    for(i = LEFT; i <= RIGHT; ++i){
        pthread_mutex_lock(&mutex_num);
        while(num != 0){
            pthread_mutex_unlock(&mutex_num);
            sched_yield();
            pthread_mutex_lock(&mutex_num);
        }
        // 如果当前分配区域空闲，则继续将任务调度到此处。
        // 在此程序中任务分配区域大小只有1
        //（初始状态下在任务等待队列较长的时候扩大任务分配区域？）
        num = i;
        pthread_mutex_unlock(&mutex_num);
    }

	// 所有任务完成
	// 在最后一个任务分配结束后，此时可能还未来的及将这个任务修改成0，所以需要循环查询num
	// 当所有任务中最后将
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
