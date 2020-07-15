//
// Created by acptek on 2020/3/17.
//

#include "rwlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void * th_r(void * p){
    rdlock_lock(p);
    puts("Read Begin");
    puts("Reading...");
    sleep(5);
    puts("Read End");
    rwlock_unlock(p);
    pthread_exit(NULL);
}

void * th_w(void * p){
    wrlock_lock(p);
    puts("Write Begin");
    puts("Writing...");
    sleep(5);
    puts("Write End");
    rwlock_unlock(p);
    pthread_exit(NULL);
}

int main()
{
    mywrlock *rw = rwlock_create();
    pthread_t t1, t2, t3, t4;

    pthread_create(&t1, NULL, th_r, rw);
    pthread_create(&t4, NULL, th_r, rw);
    pthread_create(&t2, NULL, th_w, rw);
    pthread_create(&t3, NULL, th_r, rw);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    rwlock_destroy(rw);

    exit(0);
}