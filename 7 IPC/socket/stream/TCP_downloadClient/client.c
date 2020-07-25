#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

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

    // 将socket封装成流的操作
    FILE* fp = fdopen(sfd, "r+");
    if(fp == NULL){
        perror("fdopen()");
        exit(1);
    }

    // 操作流就相当于向socket文件中写入信息
    // 发起HTTP请求, 假设目录下包含这个图片文件
    fprintf(fp, "GET /test.jpg\r\n\r\n");


    // 通过流IO进行读写
    char rbuf[BUFSIZE];
    while (1){
        size_t len = fread(rbuf, 1, BUFSIZE, fp);
        if(len <= 0)
            break;
        fwrite(rbuf, 1, len, stdout);
    }

    fclose(fp);

    // recv();
    // close();

    exit(0);
}
