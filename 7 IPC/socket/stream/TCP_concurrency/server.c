#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <time.h>
#include "proto.h"

#define IPSTRSIZE 40
#define BUFSIZE 1024

void server_job(int sfd){
    char buf[BUFSIZE];
    int len = sprintf(buf, "%lld", (long long)time(NULL));
    if(send(sfd, buf, len, 0) < 0){
        perror("send()");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    struct sockaddr_in laddr, raddr;
    int sfd;
    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket()");
        exit(1);
    }

    // 添加该选项后，再次bind时，若发现端口是未来得及释放的那么会立即释放并重新bind
//    int val = 1;
//    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){
//        perror("setsockopt()");
//        exit(1);
//    }


    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(strtol(argv[1], NULL, 10));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
    if(bind(sfd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    if(listen(sfd, 1024) < 0){
        perror("listen()");
        exit(1);
    }
    socklen_t raddr_len = sizeof(raddr);

    int newsfd;
    char ipstr[IPSTRSIZE];
    while (1) {
        if((newsfd = accept(sfd, (struct sockaddr *) &raddr, &raddr_len)) < 0){
            perror("accept()");
            exit(1);
        }

        pid_t pid;
        if((pid = fork()) < 0){
            perror("fork()");
            exit(1);
        }

        if(pid == 0){
            close(sfd);
            inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
            printf("Client: %s:%d\n", ipstr, ntohs(raddr.sin_port));
            server_job(newsfd);
            close(newsfd);
            exit(0);
        }
        close(newsfd);
    }

    close(sfd);
    exit(0);
}
