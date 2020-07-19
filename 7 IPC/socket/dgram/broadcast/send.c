#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "proto.h"

int main(int argc, char *argv[])
{
    struct msg_st *sbufp;
    struct sockaddr_in raddr;

    if(strlen(argv[1]) > NAMEMAX){
        fprintf(stderr, "Name too long\n");
        exit(1);
    }

    int size = sizeof(struct msg_st)+strlen(argv[1]);
    sbufp = malloc(size);
    if (sbufp == NULL){
        perror("malloc()");
        exit(1);
    }

    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0){
        perror("socket()");
        exit(1);
    }

    int val = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    // bind();

    strcpy(sbufp->name, argv[1]);
    sbufp->math = htonl(rand()%100);
    sbufp->chinese = htonl(rand()%100);

    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(RCVPORT));
    inet_pton(AF_INET, "255.255.255.255", &raddr.sin_addr);

    if(sendto(sd, sbufp, size, 0, (struct sockaddr *)&raddr, sizeof(raddr)) < 0){
        perror("sendto");
        exit(1);
    }
    puts("OK!");

    close(sd);

    exit(0);
}
