#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#define BUFSIZE 32

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void *fun(void *p){
    int fd = -1;
    long long n = 0;
    char buf[BUFSIZE] = "";

    fd = open(p, O_RDWR | O_CREAT, 0664);
    pthread_mutex_lock(&mutex);

    read(fd, buf, BUFSIZE);
    lseek(fd, 0, SEEK_SET);
    n = atoll(buf);

    snprintf(buf, BUFSIZE, "%lld\n", ++n);
    write(fd, buf, strlen(buf));
    
    close(fd);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main(int argc, char **argv){

    pthread_t tids[20];

    if(argc < 2){
        exit(1);
    }

    for(int i = 0; i < 20; ++i){
        pthread_create(&tids[i], NULL, fun, argv[1]);
    }

    for(int i = 0; i < 20; ++i){
        pthread_join(tids[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    exit(0);
}
