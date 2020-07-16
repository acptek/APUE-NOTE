#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

int main()
{
    int err;
    struct timespec tout;
    struct tm *tmp;
    char buf[64];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&lock);
    printf("mutex is locked\n");
    clock_gettime(CLOCK_REALTIME, &tout);
    tmp = localtime(&tout.tv_sec);
    strftime(buf, sizeof(buf), "%r", tmp);
    printf("current time : %s\n", buf);

    tout.tv_sec += 10;
    // sleep(5);
    // pthread_mutex_unlock(&lock);
    err = pthread_mutex_timedlock(&lock, &tout);

	// pthread_mutex_lock(&lock);

    clock_gettime(CLOCK_REALTIME, &tout);
    tmp = localtime(&tout.tv_sec);
    strftime(buf, sizeof(buf), "%r", tmp);
    printf("now time:%s\n", buf);

    if(err == 0){
        puts("mutex lock again");
    } else {
        printf("%s\n", strerror(err));
    }


    exit(0);
}
