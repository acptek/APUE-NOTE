#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include "relayer.h"
#define BUFSIZE 4096

enum
{
    STATE_R=1,
    STATE_W,
    STATE_Ex,
    STATE_T
};

// state machine
struct rel_fsm_st{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char *err;
    int64_t count; // write how much
};

// job
struct rel_job_st{
    int job_state; // running / cancel / over
    int fd1;
    int fd2;
    struct rel_fsm_st fsm12, fsm21;
    int fd1_save, fd2_save;
    struct timeval start, end;
};

static struct rel_job_st *rel_job[REL_JOBMAX];
static pthread_mutex_t mutex_rel_job = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

static void fsm_driver(struct rel_fsm_st *fsm)
{
    int ret;
    switch (fsm->state){
        case STATE_R:
            fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
            if(fsm->len == 0)
                fsm->state = STATE_T;
            else if (fsm->len < 0){
                if(errno == EAGAIN)
                    fsm->state = STATE_R;
                else {
                    fsm->err = "read()";
                    fsm->state = STATE_Ex;
                }
            } else {
                fsm->state = STATE_W;
                fsm->pos = 0;
            }
            break;

        case STATE_W:
            ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
            if(ret < 0){
                if(errno == EAGAIN)
                    fsm->state = STATE_W;
                else {
                    fsm->err = "write()";
                    fsm->state = STATE_Ex;
                }
            } else{ // 坚持写len个字节
                fsm->pos += ret;
                fsm->len -= ret;
                if(fsm->len == 0)
                    fsm->state = STATE_R;
                else
                    fsm->state = STATE_W;
            }
            break;

        case STATE_Ex:
            perror(fsm->err);
            fsm->state = STATE_T;
            break;

        case STATE_T:
            /* do sth. */
            break;

        default:
            abort(); // 出错人为制造一个异常得到文件
            break;
    }
}

// 负责推
static void *thr_relayer(void*){
    while(1) {
        pthread_mutex_lock(&mutex_rel_job);
        for (int i = 0; i < REL_JOBMAX; ++i) {
            if (rel_job[i] != NULL) {
                if(rel_job[i]->job_state == STATE_RUNNIG){
                    fsm_driver(&rel_job[i]->fsm12);
                    fsm_driver(&rel_job[i]->fsm21);
                    if(rel_job[i]->fsm12.state == STATE_T && rel_job[i]->fsm21.state == STATE_T)
                        rel_job[i]->job_state = STATE_OVER;
                }
            }
        }
        pthread_mutex_unlock(&mutex_rel_job);
    }
}

static void module_unload(void){

}

static void module_load(void){
    pthread_t tid_relayer;
    int err = pthread_create(&tid_relayer, NULL, thr_relayer, NULL);
    if(err){
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }

    atexit(module_unload);
}

static int get_free_pos_unlock()
{
    for (int i = 0; i < REL_JOBMAX; ++i) {
        if(rel_job[i] == NULL)
            return i;
    }
    return -1;
}

int rel_addjob(int fd1, int fd2){
    struct rel_job_st *me;

    pthread_once(&init_once, module_load);

    me = (struct rel_job_st*)malloc(sizeof(*me));
    if(me == NULL)
        return -ENOMEM;

    me->fd1 = fd1;
    me->fd2 = fd2;
    me->job_state = STATE_RUNNIG;

    me->fd1_save = fcntl(me->fd1, F_GETFL);
    fcntl(me->fd1, F_SETFL, me->fd1_save|O_NONBLOCK);
    me->fd2_save = fcntl(me->fd2, F_GETFL);
    fcntl(me->fd2, F_SETFL, me->fd2_save|O_NONBLOCK);

    me->fsm12.sfd = me->fd1;
    me->fsm12.dfd = me->fd2;
    me->fsm12.state = STATE_R;

    me->fsm21.sfd = me->fd2;
    me->fsm21.dfd = me->fd1;
    me->fsm21.state = STATE_R;

    pthread_mutex_lock(&mutex_rel_job);
    int pos = get_free_pos_unlock();
    if(pos < 0){
        pthread_mutex_unlock(&mutex_rel_job);
        fcntl(me->fd1, F_SETFL, me->fd1_save);
        fcntl(me->fd2, F_SETFL, me->fd2_save);
        free(me);
        return -ENOSPC;
    }

    rel_job[pos] = me;
    pthread_mutex_unlock(&mutex_rel_job);

    return pos;

}

#if 0
int rel_canceljob(int id);
/*
 * return   ==0     cancel success
 *          == -EINVAL
 *          == -EBUSY
 */

int rel_waitjob(int id, struct rel_stat_st *);
/*
 * return   == 0    success
 *          == -EINVAL
 *
 */

int rel_statjob(int id, struct rel_stat_st *);
/*
 * return   == 0    success
 *          == -EINVAL
 */
#endif