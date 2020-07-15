#include "mytbf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>

typedef void (*sighandler_t)(int);

/**
 * 1 这指的是一个程序可能需要分配最多 MYTBF_MAX 个任务
 * 2 这些任务可以一起进行以各自速度的流控
 * 3 是一个独占资源
 */
static struct mytbf_st * array[MYTBF_MAX];
static pthread_mutex_t mut_job = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static pthread_t tid_alarm;
static int inited = 0;
static sighandler_t alarm_handler_save;

struct mytbf_st{
    int cps;
    int burst;
    int token;
    int pos;
    pthread_mutex_t mut;
};

static void *thr_alrm(void *p){
    int i;

    // alarm(1);
    while (1) {
        pthread_mutex_lock(&mut_job);
        for (i = 0; i < MYTBF_MAX; ++i) {
            if (array[i] != NULL) {
                pthread_mutex_lock(&array[i]->mut);
                array[i]->token += array[i]->cps;
                if (array[i]->token > array[i]->burst) {
                    array[i]->token = array[i]->burst;
                }
                pthread_mutex_unlock(&array[i]->mut);
            }
        }
        pthread_mutex_unlock(&mut_job);
        sleep(1); // change
    }
}

static void module_unload(void){
    int i;
    // signal(SIGALRM, alarm_handler_save);
    // alarm(0);

    pthread_cancel(tid_alarm);
    pthread_join(tid_alarm, NULL);

    for(i = 0; i < MYTBF_MAX; ++i) {
        if(array[i] != NULL){
            mytbf_destroy(array[i]);
        }
    }

    pthread_mutex_destroy(&mut_job);
}

/**
 * 发出第一个信号的模块加载
 */
static void module_load(void){
    // alarm_handler_save = signal(SIGALRM, alrm_handler);
    // alarm(1);
    int err;

    err = pthread_create(&tid_alarm, NULL, thr_alrm, NULL);
    if(err){
        fprintf(stderr, "pthread_create:%s\n", strerror(err));
        exit(1);
    }

    atexit(module_unload);
}

mytbf_t * mytbf_init(int cps, int burst){
    int i;
    struct mytbf_st *me;

    pthread_once(&init_once, module_load);

    me = malloc(sizeof(struct mytbf_st));
    if(me == NULL)
        return NULL;
    me->cps = cps;
    me->burst = burst;
    me->token = 0;
    pthread_mutex_init(&me->mut, NULL);

    pthread_mutex_lock(&mut_job);

    // find the first pos which is not occupied
    for(i = 0; i < MYTBF_MAX; i++){
        if(array[i] == NULL){
            break;
        }
    }
    if(i >= MYTBF_MAX){
        pthread_mutex_unlock(&mut_job);
        free(me);
        return NULL;
    }
    // init
    me->pos = i;
    array[i] = me;
    pthread_mutex_unlock(&mut_job);

    return me;
}

int mytbf_fetchtoken(mytbf_t *tbf, int size){
    int n;
    struct mytbf_st *me = tbf;
    if(size <= 0)
        return -EINVAL;


    pthread_mutex_lock(&me->mut);
    while (me->token <= 0){
        pthread_mutex_unlock(&me->mut);
        sched_yield();
        pthread_mutex_lock(&me->mut);
    }
    n = me->token < size ? me->token : size;
    me->token -= n;
    pthread_mutex_unlock(&me->mut);
    return n;
}

int mytbf_returntoken(mytbf_t *tbf, int size){
    struct mytbf_st *me = tbf;
    if(size <= 0)
        return -EINVAL;

    pthread_mutex_lock(&me->mut);
    me->token += size;
    if(me->token > me->burst)
        me->token = me->burst;
    pthread_mutex_unlock(&me->mut);

    return size;
}

int mytbf_destroy(mytbf_t *tbf){
    struct mytbf_st *me = tbf;
    pthread_mutex_lock(&mut_job);
    array[me->pos] = NULL;
    pthread_mutex_unlock(&mut_job);
    pthread_mutex_destroy(&me->mut);
    free(tbf);

    return 0;
}


/**
 * 查询法 -> 通知法
 * 先处于等待状态，当上层加了token后告知
 * 信号唤醒
 */
