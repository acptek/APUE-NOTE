#include "mypipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// 管道数据结构
struct mypipe_st{
    int head;
    int tail;
    char data[PIPESIZE];
    int datasize; //管道中的有效字节数
    int count_rd; // 读者计数
    int count_wr; // 写者计数
    pthread_mutex_t mut;
    pthread_cond_t cond;
};

mypipe_t * mypipe_init(void){
    struct mypipe_st *me;
    me = malloc(sizeof(struct mypipe_st*));
    if(me == NULL) return NULL;

    me->head = 0;
    me->tail = 0;
    me->datasize = 0;
    me->count_rd = 0;
    me->count_wr  =0;
    pthread_mutex_init(&me->mut, NULL);
    pthread_cond_init(&me->cond, NULL);

    return me;
}

/*
 * 在管道中，一对构成读写，若二者中只有一种，那么进行等待另一个的到来
 */
int mypipe_register(mypipe_t *ptr, int opmap){
    struct mypipe_st *me = ptr;
//    if(!(opmap&MYPIPE_READ || opmap&MYPIPE_WRITE))
//        return -1;
    pthread_mutex_lock(&me->mut);
    if(opmap & MYPIPE_READ)
        me->count_rd++;
    if(opmap & MYPIPE_WRITE)
        me->count_wr++;
    // 当读者或者写者到来时，叫醒其他等待的，注册自己身份的人
    // 读者或写者凑齐时，则进行
    pthread_cond_broadcast(&me->cond); // 多个线程同时操作后，进行通知，想要凑齐一对读者写者
    while(me->count_rd <= 0 || me->count_wr <= 0)
        pthread_cond_wait(&me->cond, &me->mut);

    pthread_mutex_unlock(&me->mut);
    return 0;
}

int mypipe_unregister(mypipe_t *ptr, int opmap){

    struct mypipe_st *me = ptr;

    pthread_mutex_lock(&me->mut);
    if(opmap & MYPIPE_READ)
        me->count_rd--;
    if(opmap & MYPIPE_WRITE)
        me->count_wr--;
    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);

    return 0;
}

static int mypipe_readbyte_unlocked(struct mypipe_st *me, char *datap){
    if(me->datasize <= 0)
        return -1;
    * datap = me->data[me->head];
    me->head = (me->head+1)%(PIPESIZE);
    me->datasize --;
    return 0;
}

int mypipe_read(mypipe_t *ptr, void *buf, size_t count){
    struct mypipe_st *me = ptr;
    int i;
    pthread_mutex_lock(&me->mut);

    // 管道为空且有写者那么 读才等待， 否则1-管道空直接读了0然后返回 2-管道非空直接进行读
    while (me->datasize <= 0 && me->count_wr > 0)
        pthread_cond_wait(&me->cond, &me->mut);

    // 对应情况 1
    if(me->datasize <= 0 && me->count_wr <= 0){
        pthread_mutex_unlock(&me->mut);
        return 0;
    }
    // 对应情况 2
    for (i = 0; i < count; ++i) {
        if(mypipe_readbyte_unlocked(me, buf+i) != 0) // 一次调用读一个字节
            break;
    }
    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);

    return i;
}

static int mypipe_writebyte_unlocked(struct mypipe_st *me, const char *datap){
    if(me->datasize >= PIPESIZE)
        return -1;
    me->data[me->tail] = *datap;
    me->tail = (me->tail+1)%(PIPESIZE);
    me->datasize ++;
    return 0;
}

int mypipe_write(mypipe_t *ptr, const void *buf, size_t count){
    struct mypipe_st *me = ptr;
    int i;
    pthread_mutex_lock(&me->mut);
    while(me->datasize >= PIPESIZE && me->count_rd > 0)
        pthread_cond_wait(&me->cond, &me->mut);
    if(me->datasize >= PIPESIZE && me->count_rd <= 0){
        pthread_mutex_unlock(&me->mut);
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if(mypipe_writebyte_unlocked(me, buf+i) != 0)
            break;
    }
    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);

    return i;
}

int mypipe_destroy(mypipe_t *ptr){
    struct mypipe_st *me = ptr;

    pthread_mutex_destroy(&me->mut);
    pthread_cond_destroy(&me->cond);

    free(ptr);
    return 0;
}
