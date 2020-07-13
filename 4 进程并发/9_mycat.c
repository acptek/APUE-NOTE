#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define BUFSIZE 10
#define BURST   100

static volatile int token = 0;

static void alrm_handler(int s){
    alarm(1);
    token++;
    if(token > BURST)
        token = BURST;
}

int main(int argc, char **argv){

    int sfd, dfd = 1;
    int len, ret, pos;
    char buf[BUFSIZE];

    if(argc < 2){
        fprintf(stderr, "Usage Error ...\n");
        exit(1);
    }

    signal(SIGALRM, alrm_handler);

    alarm(1);

    do{
        sfd = open(argv[1], O_RDONLY);
        if(sfd < 0){
            // not interpret
            if(errno != EINTR){
                perror("Open Error");
                exit(1);
            }
        }
    }while (sfd < 0);

    while (1){
        while (token <= 0)
            pause();

        token --;

        while((len = read(sfd, buf, BUFSIZE)) < 0) {
            if (errno == EINTR)
                continue;
            perror("Read Error");
            break;
        }
        if(len == 0)
            break;
        pos = 0;
        while (len > 0){
            ret = write(dfd, buf+pos, len);
            if(ret < 0){
                if(errno == EINTR)
                    continue;
                perror("Write Error");
                exit(1);
            }
            pos += ret;
            len -= ret;
        }
        sleep(1);
    }

    close(sfd);

    exit(0);
}
