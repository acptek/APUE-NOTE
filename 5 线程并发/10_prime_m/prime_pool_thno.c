// prmie_pool的通知法版本

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
static pthread_cond_t  cond_num = PTHREAD_COND_INITIALIZER;

static void *thr_prime(void *p){

    int i, j, mark;

    while (1) {
        pthread_mutex_lock(&mutex_num);
        while (num == 0) {
            pthread_cond_wait(&cond_num, &mutex_num);
//            pthread_mutex_unlock(&mutex_num);
//            sched_yield();
//            pthread_mutex_lock(&mutex_num);
        }
        if(num == -1) {
            pthread_mutex_unlock(&mutex_num);
            break;
        }
        i = num;
        num = 0;
        pthread_cond_broadcast(&cond_num); // 在一个任务分配结束后，通知分配区域，有线程即将空闲
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
    // 先创建好执行线程
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
    
    // 和查询法一样，num=0表示当前任务分配区域为空，等待任务的到来或者需要获取任务
    // num!=0表示已经有任务在区域中等待分派
    // 但是需要线程来通知任务分配区域有空闲线程可以处理任务了
    // 相比于查询法不是 不停地去查看任务区域是否空闲
    for(i = LEFT; i <= RIGHT; ++i){
        pthread_mutex_lock(&mutex_num);
        while(num != 0){
            pthread_cond_wait(&cond_num, &mutex_num);
        }
        num = i;
        pthread_cond_signal(&cond_num);
        pthread_mutex_unlock(&mutex_num);
    }

	// 在所有任务分配完成后，通过不断查询是否所有任务已经分配
	// 只是在最后任务结束阶段查询，不会对性能造成很大影响
    pthread_mutex_lock(&mutex_num);
    while(num != 0){
        pthread_mutex_unlock(&mutex_num);
        sched_yield();
        pthread_mutex_lock(&mutex_num);
    }
    num = -1;
    pthread_cond_broadcast(&cond_num);
    pthread_mutex_unlock(&mutex_num);

    for(i = 0; i < THRNUM; ++i){
        pthread_join(tid[i], &ptr);
        free(ptr);
    }

    pthread_mutex_destroy(&mutex_num);
    pthread_cond_destroy(&cond_num);

    exit(0);
}
