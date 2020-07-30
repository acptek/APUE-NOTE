#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <errno.h>
#include <time.h>

#define MINSPARESERVER 5
#define MAXSPARESERVER 10
#define MAXCLIENT 20
#define LINEBUFSIZE 1024
#define IPSTRSIZE 40
#define SERVERPORT "1989"

#define SIG_NOTIFY SIGUSR2

enum {
    STATE_IDEL = 0,
    STATE_BUSY
};

// 表示一个子进程server端
struct server_st{
    pid_t pid;
    int state;
    // int reuse; // 重用计数，达到一定数量杀死进程重新创建进程
};

static struct server_st *serverpool;
static int idle_count = 0, busy_count = 0;
int sfd;

// 子进程通知父进程的SIG_NOTIFY(SIGUSR2)信号处理函数
void usr2_handler(int s){
    // pass;
}

void server_job(int pos){
    char linebuf[LINEBUFSIZE];
    struct sockaddr_in raddr;
    socklen_t raddr_len;
    int client_sfd;
    // char ipstr[IPSTRSIZE];

    pid_t ppid = getppid();
    while(1){
        serverpool[pos].state = STATE_IDEL;
        // 状态变化，发送信号
        kill(ppid, SIG_NOTIFY);
        client_sfd = accept(sfd, (void *)&raddr, &raddr_len);
        if(client_sfd < 0){
            if(errno != EINTR || errno != EAGAIN) {
                perror("accept()");
                exit(1);
            }
        }

        serverpool[pos].state = STATE_BUSY;
        // 状态变化，发送信号
        kill(ppid, SIG_NOTIFY);
        // inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSZIE);
        // printf("[%d]client:%s:%d%\n", getpid(), ipstr, ntohs(raddr.sin_port));
        long long stamp = time(NULL);
        int len = snprintf(linebuf, LINEBUFSIZE, "%lld", stamp);
        send(client_sfd, linebuf, len, 0);
        sleep(5);
        close(client_sfd);
    }
}

int add_1_server(void){
    // 超过最大值则不增加
    if(idle_count + busy_count >= MAXCLIENT)
        return -1;
    // 有空位可以添加server端
    int i;
    for(i = 0; i < MAXCLIENT; ++i){
        if(serverpool[i].pid == -1)
            break;
    }
    serverpool[i].state = STATE_IDEL;
    pid_t pid = fork();
    if(pid < 0){
        perror("fork()");
        exit(1);
    }
    // 子进程中server后退出，父进程中设置全局状态
    if(pid == 0){
        server_job(i);
        exit(0);
    } else {
        serverpool[i].pid = pid;
        idle_count++;
    }
    return 0;
}

int del_1_server(void){
    if(idle_count == 0)
        return -1;
    for(int i = 0; i < MAXCLIENT; ++i){
        if(serverpool[i].pid != -1 && serverpool[i].state == STATE_IDEL){
            kill(serverpool[i].pid, SIGTERM);
            serverpool[i].pid = -1;
            idle_count--;
            break;
        }
    }
    return 0;
}

int scan_pool(void){
    int idle = 0, busy = 0;
    for(int i = 0; i < MAXCLIENT; ++i){
        if(serverpool[i].pid == -1)
            continue;
        // 发送信号0，检测当前继承是否存在
        if(kill(serverpool[i].pid, 0)){
            serverpool[i].pid = -1;
            continue;
        }
        if(serverpool[i].state == STATE_IDEL){
            idle++;
        } else if (serverpool[i].state == STATE_BUSY) {
            busy++;
        } else {
            fprintf(stderr, "Unknown state.\n");
            abort();
        }
    }
    idle_count = idle;
    busy_count = busy;
}

int main(int argc, char **argv)
{
    struct sigaction sa, oldsa;
    // 创建子进程后自行消亡
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, &oldsa);

    // 注册信号通知
    sa.sa_handler = usr2_handler;
    sigemptyset(&sa.sa_mask); // 清空sa_mask，且与其他信号不共享处理函数无需sigadd
    sa.sa_flags = 0;
    sigaction(SIG_NOTIFY, &sa, &oldsa);

    // 屏蔽信号，防止重入；在模块（程序）结束后恢复
    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, SIG_NOTIFY);
    // 阻塞set中的信号：SET_NOTIFY
    sigprocmask(SIG_BLOCK, &set, &oldset);

    // 申请 MAXCLIENT 个 server_st 空间 //使用mmap代替malloc
    serverpool = mmap(NULL, sizeof(struct server_st)*MAXCLIENT, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(serverpool == MAP_FAILED){
        perror("mmap()");
        exit(1);
    }

    // init
    for(int i = 0; i < MAXCLIENT; ++i){
        serverpool[i].pid = -1;
    }

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0){
        perror("socket()");
        exit(1);
    }

    // 设置关闭端口可重用选项
    int val = 1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){
        perror("setsockopt()");
        exit(1);
    }

    struct sockaddr_in laddr;
    laddr.sin_family = AF_INET;
    laddr.sin_port =htons(strtol(SERVERPORT, NULL, 10));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
    if(bind(sfd, (void *)&laddr, sizeof(laddr)) < 0){
        perror("bind()");
        exit(1);
    }

    // 100: 支持全连接的个数
    if(listen(sfd, 100) < 0){
        perror("listen()");
        exit(1);
    }

    // 添加最少空闲进程个数
    for(int i = 0; i < MINSPARESERVER; ++i){
        // 在add_1_server中创建子进程，子进程中创建连接，然后进行服务
        add_1_server();
    }

    while (1){
        // 进行信号驱动，等待来自子进程的SIG_NOTIFY信号
        sigsuspend(&oldset); // 变成oldset状态，等待信号，恢复信号状态
        // control
        // 接收到一个信号后（即子进程状态发生变化时），对信号响应，遍历子进程状态数组，更改全局状态
        scan_pool();

        if(idle_count > MAXSPARESERVER){ // 如果超过最大空闲进程数，就删除到最大空闲进程数
            for(int i = 0; i < (idle_count - MAXSPARESERVER); ++i){
                del_1_server();
            }
        } else if(idle_count < MINSPARESERVER) { // 如果小于最小空闲进程数，就增加到
            for(int i = 0; i < (MINSPARESERVER - idle_count); ++i){
                add_1_server();
            }
        }

        // 输出当前池的状态
        for(int i = 0; i < MAXCLIENT; ++i){
             if(serverpool[i].pid == -1)
                 putchar('-');
             else if (serverpool[i].state == STATE_IDEL)
                 putchar('.');
             else if (serverpool[i].state == STATE_BUSY)
                 putchar('x');
        }
        putchar('\n');
    }

    sigprocmask(SIG_SETMASK, &oldset, NULL); // 恢复原来MASK

    exit(0);
}
