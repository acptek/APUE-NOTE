#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "proto.h"
#include <net/if.h>
#define IPSIZE 40

int main()
{
    struct sockaddr_in laddr, raddr;
    struct msg_st *rbufp;
    socklen_t raddr_len;
    char ipstr[IPSIZE];

    int size = sizeof(struct msg_st) + NAMEMAX - 1;
    rbufp = malloc(size);
    if(rbufp == NULL){
        perror("malloc()");
        exit(1);
    }

    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0){
        perror("socket()");
        exit(1);
    }

	// 创建多播组，根据IP_ADD_MEMBERSHIP选项设置optval参数
    struct ip_mreqn mreqn;
    inet_pton(AF_INET, MGROUP, &mreqn.imr_multiaddr);
    inet_pton(AF_INET, "0.0.0.0", &mreqn.imr_address);
    mreqn.imr_ifindex = if_nametoindex("enp1s0"); // 当前使用的网络设备的索引号

	// 设置选项
    if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreqn, sizeof(mreqn)) < 0){
        perror("setsockopt()");
        exit(1);
    }


    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(RCVPORT));
    // 0.0.0.0 绑定当前IP
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);

    if(bind(sd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    // 必须初始化
    raddr_len = sizeof(raddr);
    while(1) {
        puts("Start ...");
        recvfrom(sd, rbufp, size, 0, (void *)&raddr, &raddr_len);

        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSIZE);
        printf("---MESSAGE FROM %s:%d---\n", ipstr, ntohs(raddr.sin_port));
        printf("NAME = %s\n", rbufp->name);
        printf("MATH = %d\n", ntohl(rbufp->math));
        printf("CHINESE = %d\n", ntohl(rbufp->chinese));

        puts("End");
    }

    close(sd);

    exit(0);
}
