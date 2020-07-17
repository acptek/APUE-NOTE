#include "anytimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#define JobMax  1024
#define INIT    0
#define RUNN    1
#define PAUSE   2
#define FINISH  3
#define CANCEL  4

struct Job{
    int sec;
    int flag; // 0: null  1: run  2: pause  3: finish  4: canceled
    void * func;
    char * arg;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

static struct Job * Tasks[JobMax];

pthread_mutex_t TaskMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_once_t Once = PTHREAD_ONCE_INIT;
pthread_t tid;

void * th_fun(void *p){
    while(1) {
        pthread_mutex_lock(&TaskMutex);
        for (int i = 0; i < JobMax; ++i) {
            pthread_mutex_lock(&Tasks[i]->mutex);
            if (Tasks[i] != NULL && Tasks[i]->sec > 0) {
                Tasks[i]->sec--;
            }
            if (Tasks[i]->sec == 0) {
                pthread_cond_broadcast(&Tasks[i]->cond);
            }
            pthread_mutex_unlock(&Tasks[i]->mutex);
        }
        pthread_mutex_unlock(&TaskMutex);
        nanosleep(NULL, NULL);
    }
}

void * unmodule(){

    pthread_cancel(tid);
    pthread_join(tid, NULL);

    for (int i = 0; i < JobMax; ++i) {
        Tasks[i] = NULL;
        pthread_mutex_destroy(&Tasks[i]->mutex);
        pthread_cond_destroy(&Tasks[i]->cond);
    }
    pthread_mutex_destroy(&TaskMutex);
}

void * module(){
    pthread_create(&tid, NULL, th_fun, NULL);

    atexit(unmodule());
}

// arg 的操作方式 ？
// 什么时候传指针是么时候传int
int at_addjob(int sec, at_jobfunc_t *jobp, void *arg){
    pthread_once(&Once, module);

    pthread_mutex_lock(&TaskMutex);
    int i;
    for(i = 0; i < JobMax; ++i){
        if(Tasks[i] == NULL){
            break;
        }
    }
    if(i >= JobMax){
        return -ENOSPC;
    }
    pthread_mutex_init(&Tasks[i]->mutex, NULL);
    pthread_cond_init(&Tasks[i]->cond, NULL);
    Tasks[i]->sec = sec;
    Tasks[i]->func = jobp;
    Tasks[i]->arg = arg;
    Tasks[i]->flag = 0;
    return i;
}

///*
// * return == 0       success, 成功指定任务成功取消
// *        == -EINVAL fail, 参数非法
// *        == -EBUSY  fail, 指定任务已完成
// *        == ECANCELED fail, 指定任务重复取消
// */
//int at_canceljob(int id){
//    if(Tasks[id]->flag == CANCEL)
//        return -ECANCELED;
//    else if (Tasks[id]->flag == FINISH)
//        return -EBUSY;
//    Tasks[id]->flag = CANCEL;
//    Tasks[id]->sec = 0;
//    return 0;
//}


// 回收
int at_waitjob(int id){
    pthread_mutex_lock(&Tasks[id]->mutex);
    while (Tasks[id]->sec > 0){
        pthread_cond_wait(&Tasks[id]->cond, &Tasks[id]->mutex);
    }
    pthread_mutex_unlock(&Tasks[id]->mutex);
    return 0;
}

//
//int at_pausejob(int id){
//    Tasks[id]->flag = PAUSE;
//    return 0;
//}
//
//int at_resumejob(int id){
//    Tasks[id]->flag = RUNN;
//}
