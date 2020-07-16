// 使用sigaction来代替alarm

#include "mytbf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

//typedef void (*sighandler_t)(int);

static struct mytbf_st * array[MYTBF_MAX];
static int inited = 0;
//static sighandler_t alarm_handler_save;
static struct sigaction alrm_sa_save;

struct mytbf_st{
    int cps;
    int burst;
    int token;
    int pos;
};

static void alrm_action(int s, siginfo_t *infop, void *unused){
    int i;

    if(infop->si_code != SI_KERNAL){
        return;
    }

    //alarm(1);

    for(i = 0; i < MYTBF_MAX; ++i){
        if(array[i] != NULL){
            array[i]->token += array[i]->cps;
            if(array[i]->token > array[i]->burst){
                array[i]->token = array[i]->burst;
            }
        }
    }
}

static void module_unload(void){
    int i;
    //signal(SIGALRM, alarm_handler_save);
    //alarm(0);


    struct itimerval itv;

    sigaction(SIGALRM, &alrm_sa_save, NULL);

    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 0;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, NULL);

    for(i = 0; i < MYTBF_MAX; ++i)
        free(array[i]);
}

/**
 * 发出第一个信号的模块加载
 */
static void module_load(void){
    //alarm_handler_save = signal(SIGALRM, alrm_handler); // sigaction
    //alarm(1); // -> setitimer
    struct sigaction sa;
    struct itimerval itv;

    sa.sa_sigaction = alrm_action;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGALRM, &sa, &alrm_sa_save);
    /*
     * if error
     */
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, NULL);
    /*
     * if error
     */

    atexit(module_unload);
}

mytbf_t * mytbf_init(int cps, int burst){
    int i;
    struct mytbf_st *me;

    if(!inited) {
        module_load();
        inited = 1;
    }

    me = malloc(sizeof(struct mytbf_st));
    if(me == NULL)
        return NULL;

    for(i = 0; i < MYTBF_MAX; i++){
        if(array[i] == NULL){
            break;
        }
    }
    if(i >= MYTBF_MAX)
        return NULL;
    me->pos = i;
    me->cps = cps;
    me->burst = burst;
    me->token = 0;
    array[i] = me;

    return me;
}

int mytbf_fetchtoken(mytbf_t *tbf, int size){
    int n;
    struct mytbf_st *me = tbf;
    if(size <= 0)
        return -EINVAL;

    while (me->token <= 0)
        pause;

    n = me->token < size ? me->token : size;

    me->token -= n;
    return n;
}

int mytbf_returntoken(mytbf_t *tbf, int size){
    struct mytbf_st *me = tbf;
    if(size <= 0)
        return -EINVAL;

    me->token += size;
    if(me->token > me->burst)
        me->token = me->burst;

    return size;
}

int mytbf_destroy(mytbf_t *tbf){
    struct mytbf_st *me = tbf;
    array[me->pos] = NULL;
    free(tbf);

    return 0;
}
