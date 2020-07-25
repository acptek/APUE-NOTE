#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{

    int sfd;
    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(1);
    }

    // bind();

    struct sockaddr_in raddr;
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(strtol(argv[2], NULL, 10));
    inet_pton(AF_INET, argv[1], &raddr.sin_addr);
    if(connect(sfd, (struct sockaddr *)&raddr, sizeof(raddr)) < 0){
        perror("connect()");
        exit(1);
    }

    FILE* fp = fdopen(sfd, "r+");
    if(fp == NULL){
        perror("fdopen()");
        exit(1);
    }

    long long stamp;
    if(fscanf(fp, "%lld", &stamp) < 1){
        fscanf(stderr, "Bad Format\n");
    } else {
        fprintf(stdout, "stamp = %lld\n", stamp);
    }

    fclose(fp);

    // recv();
    // close();

    exit(0);
}
