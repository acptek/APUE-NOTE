#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "mytbf.h"
#include <unistd.h>

#define CPS     10
#define BUFSIZE 1024
#define BURST   100



int main(int argc, char **argv){
    int sfd, dfd = 1;
    char buf[BUFSIZE];
    int ret, pos;
    ssize_t len = 0;
    mytbf_t *tbf;

    tbf = mytbf_init(CPS, BURST);
    if(tbf == NULL){
        perror("mytbf_init()");
        exit(1);
    }

    do{
        sfd = open(argv[1], O_RDONLY);
        if(sfd < 0){
            if(errno == EINTR)
                continue;
            perror("open()");
            exit(1);
        }
    }while(sfd < 0);

    while(1){
        size_t size = mytbf_fetchtoken(tbf, BUFSIZE);
        while((len = read(sfd, buf, size)) < 0){
            if(errno == EINTR) continue;
            perror("read()");
            break;
        }
        if(len == 0)
            break;
        pos = 0;
        if(size - len > 0)
            mytbf_returntoken(tbf, size-len); 

        while(len > 0){
            ret = write(dfd, buf+pos, len);
            if(ret < 0){
                if(errno == EINTR)
                    continue;
                perror("write()");
                exit(1);
            }
            pos += ret;
            len -= ret;
        }
    }

    close(sfd);
    mytbf_destroy(tbf);

    exit(0);
}
