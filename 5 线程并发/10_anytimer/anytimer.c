#include "anytimer.h"
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#define JOB_MAX 1024

enum {
	STATE_RUNNING = 1,
	STATE_CANCELED,
	STATE_OVER
};

struct at_job_st {
	int job_state;
	int sec; // sec秒后执行jobp函数，参数是arg
	int time_remain; // 执行时代替sec使用，防止出现错误sec已经变化了
	// int repeat; // 是否是周期性任务
	at_jobfunc_t *jobp;
	void *arg; //传参
};

static struct at_job_st * job[JOB_MAX];
static int inited = 0;

struct sigaction alrm_sa_save;

// 只接收从内核中发出的信号
void alrm_action (int s, siginfo_t *infop, void *unused){
	if(infop->si_code != SI_KERNEL)
		return ;
	for(int i = 0; i < JOB_MAX; ++i){
		if(job[i] != NULL && job[i]->job_state != STATE_RUNNING){
			job[i]->time_remain --;
			if(job[i]->time_remain == 0){
				job[i]->jobp(job[i]->arg);
				job[i]->job_state = STATE_OVER;
			}
		}
	}
}

void module_unload(void){
	struct itimerval itv;
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &itv, NULL);
	if(sigaction(SIGALRM, &alrm_sa_save, NULL) < 0){
		perror("sigaction()");
		exit(1);
	}
}

void module_load(void){
	struct sigaction sa;
	struct itimerval itv;
	sa.sa_sigaction = alrm_action;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	if(sigaction(SIGALRM, &sa, &alrm_sa_save) < 0){
		perror("sigaction()");
		exit(0);
	}
	
	itv.it_interval.tv_sec = 1;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	if(setitimer(ITIMER_REAL, &itv, NULL) < 0){
		perror("setitimer()");
		exit(1);
	}

	atexit(module_unload);
}

static int get_free_pos(void){
	for(int i = 0; i < JOB_MAX; ++i){
		if(job[i] == NULL)
			return i;
	}
	return -1;
}

// arg 的操作方式 ？
int at_addjob(int sec, at_jobfunc_t *jobp, void *arg){

	int pos;
	struct at_job_st *me;
	
	if(!inited){
		module_load();
		inited = 1;
	}
	
	pos = get_free_pos();
	if(pos < 0){
		return -ENOSPC;
	}
	
    me = malloc(sizeof(*me));
    if(me == NULL)
    	return -ENOMEM;
    me->job_state = STATE_RUNNING;
    me->sec = sec;
    me->jobp = jobp;
    me->arg = arg;
    
    job[pos] = me;
    return pos;

}

int at_canceljob(int id){
	if(id < 0 || id >= JOB_MAX || job[id] == NULL)
		return -EINVAL;
	if(job[id]->job_state == STATE_CANCELED)
		return - ECANCELED;
	if(job[id]->job_state == STATE_OVER)
		return -EBUSY;
	job[id]->job_state = STATE_CANCELED;
	
	return 0;
}

int at_waitjob(int id){
	if(id < 0 || id >= JOB_MAX || job[id] == NULL)
		return -EINVAL;
	while(job[id]->job_state == STATE_RUNNING)
		pause();
	if(job[id]->job_state == STATE_CANCELED || job[id]->job_state == STATE_OVER){
		free(job[id]);
		job[id] = NULL;
	}
	
	return 0;
}

int at_pausejob(int id){

}

int at_resumejob(int id){

}
