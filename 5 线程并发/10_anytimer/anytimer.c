#include "anytimer.h"
#include <stdio.h>
#include <stdlib.h>
#define JobMax 1024

struct Job{
    int sec;
    void * func;
    char * arg;
};

static struct Job * Tasks[JobMax];

// arg 的操作方式 ？
int at_addjob(int sec, at_jobfunc_t *jobp, void *arg){
    struct Job *me = malloc(sizeof(Job));
    me->sec = sec;
    me->func = jobp;
    me->arg = arg;


}

int at_canceljob(int id){

}

int at_waitjob(int id){

}

int at_pausejob(int id){

}

int at_resumejob(int id){

}
