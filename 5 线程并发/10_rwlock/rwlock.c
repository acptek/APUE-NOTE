//
// Created by acptek on 2020/3/17.
//

#include "rwlock.h"
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

struct rwlock{
    int count;
    int waitrd;
    int waitwr;
    pthread_mutex_t mutex;
    pthread_cond_t cond_r;
    pthread_cond_t cond_w;
};

mywrlock * rwlock_create(void){
    struct rwlock *me = malloc(sizeof(struct rwlock));
    me->count = 0;
    me->waitrd = 0;
    me->waitwr = 0;
    pthread_mutex_init(&me->mutex, NULL);
    pthread_cond_init(&me->cond_r, NULL);
    pthread_cond_init(&me->cond_w, NULL);
    return me;
}

int rdlock_lock(mywrlock *lock){
    struct rwlock *me = lock;
    pthread_mutex_lock(&me->mutex);
    while (me->count < 0 || me->waitwr > 0){
        me->waitrd++;
        pthread_cond_wait(&me->cond_r, &me->mutex);
        me->waitrd--;
    }
    me->count++;
    pthread_mutex_unlock(&me->mutex);
    return 0;
}

int wrlock_lock(mywrlock * lock){
    struct rwlock *me = lock;
    pthread_mutex_lock(&me->mutex);
    while (me->count != 0){
        me->waitwr++;
        pthread_cond_wait(&me->cond_w, &me->mutex);
        me->waitwr--;
    }
    me->count = -1;
    pthread_mutex_unlock(&me->mutex);
    return 0;
}

int rwlock_unlock(mywrlock *lock){
    struct rwlock *me = lock;
    pthread_mutex_lock(&me->mutex);
    if(me->count == -1){
        me->count = 0;
        if(me->waitwr > 0){
            pthread_cond_signal(&me->cond_w);
        } else if(me->waitrd > 0){
            pthread_cond_broadcast(&me->cond_r);
        }
    } else if(me->count > 0){
        me->count--;
        if(me->waitrd > 0)
            pthread_cond_signal(&me->cond_w);
    }
    pthread_mutex_unlock(&me->mutex);
    return 0;
}

int rdlock_trylock(mywrlock *lock){
    struct rwlock *me = lock;
    pthread_mutex_lock(&me->mutex);
    if (me->count < 0 || me->waitwr > 0){
        pthread_mutex_unlock(&me->mutex);
        return EBUSY;
    }
    me->count++;
    pthread_mutex_unlock(&me->mutex);
    return 0;
}

int wrlock_trylock(mywrlock * lock){
    struct rwlock *me = lock;
    pthread_mutex_lock(&me->mutex);
    if (me->count != 0){
        pthread_mutex_unlock(&me->mutex);
        return EBUSY;
    } else {
        me->count = -1;
    }
    pthread_mutex_unlock(&me->mutex);
    return 0;
}

int rwlock_destroy(mywrlock * lock){
    struct rwlock *me = lock;
    pthread_mutex_lock(&me->mutex);
    if(me->count != 0 || me->waitwr > 0){
        pthread_mutex_unlock(&me->mutex);
        return -1;
    }
    pthread_mutex_unlock(&me->mutex);
    pthread_mutex_destroy(&me->mutex);
    pthread_cond_destroy(&me->cond_r);
    pthread_cond_destroy(&me->cond_w);
    free(me);
    return 0;
}