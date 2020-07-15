#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <zconf.h>

#define FNAME "/etc/service"
#define TMPNUM  10
#define THNUM   4

static int thnum = 0;
pthread_mutex_t th_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t th_cond = PTHREAD_COND_INITIALIZER;
struct Lock rdlock, wrlock;

struct Lock{
    int num;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void init_lock(struct Lock *lock){
    lock->num = 0;
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->cond, NULL);
}

void read_lock(void){
    pthread_mutex_lock(&rdlock.mutex);
    rdlock.num ++;
    if(rdlock.num == 1){
        pthread_mutex_lock(&wrlock.mutex);
    }
    pthread_mutex_unlock(&rdlock.mutex);
}

void read_unlock(void){
    pthread_mutex_lock(&rdlock.mutex);
    rdlock.num --;
    if(rdlock.num == 0){
        pthread_mutex_unlock(&wrlock.mutex);
    }
    pthread_mutex_unlock(&rdlock.mutex);
}

void write_lock(void){
    pthread_mutex_lock(&wrlock.mutex);
}

void write_unlock(void){
    pthread_mutex_unlock(&wrlock.mutex);
}

void init_rwlock(struct Lock *rl, struct Lock *wl){
    init_lock(rl);
    init_lock(wl);
}

void Read(void){
    puts("Start Reading");
    sleep(5);
    puts("End Reading");
}

void Write(void){
    puts("Start Writing");
    sleep(5);
    puts("End Writing");
}


int main()
{
    init_rwlock(&rdlock, &wrlock);
    int rwqueue[TMPNUM] = {0, 0, 1, 0, 1, 1, 0, 0, 1, 0};

    for(int i = 0; i < TMPNUM; ++i){

    }

    exit(0);
}



/**
 * 分析
 * 1 首先需要控制线程的数量，也就是在创建线程前检测是否还能创建（当前的线程数是否达到规定上限
 *      -> thnum , mutex_thnum , cond_thnum
 * 2 对于每一个新到来的线程，分为读线程和写线程
 *      2.1 读线程：设置一个读线程的计数
 *          读线程的计数包括：当前在读，等待读
 *          a.（当一个线程在读，另一个写线程请求访问时，再到来一个读线程则将其设置为等待读）
 *          b. (当一个线程在读，没有写线程等待，则当前在读线程计数加1，开始读，读完计数减1)
 *          c. (当一个线程在写，读线程到来时设置为等待)
 *
 */

// 100 r  // 100 w // 2 // 2
