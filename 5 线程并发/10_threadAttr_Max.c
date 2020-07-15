#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>

void *func(void *p){
    while (1)
        pause();
    pthread_exit(NULL);
}

int main()
{

    int i, err;
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1000*1024*1024);

    for (i = 0;  ; ++i) {
        err = pthread_create(&tid, &attr, func, NULL);
        //printf("%d\n", i);
        if(err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            break;
        }
    }

    printf("--> %d\n", i);

    pthread_attr_destroy(&attr);

    exit(0);
}

/*
 * 1 尝试将栈空间开到1G，仍然是1w1k+ -》 11T
 * 想再继续家到 10G，30G，则会超出size_t的范围 : __SIZE_TYPE__
 * typedef  unsigned long size_t;
 * typedef  unsigned int size_t;
 * Visual C++和Mingw64字节数为4字节
 *
 * 2 单个进程虚拟内存用尽后才会出现只能create出几十个线程的情况
 *
 * 3 物理内存的大小，多线程用尽物理内存
 */
