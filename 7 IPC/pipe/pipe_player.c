#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

#define BUFSIZE 1024

int main()
{
    char buf[BUFSIZE];
    int pd[2];
    pid_t pid;
    int len;

    if(pipe(pd)){
        perror("pipe()");
        exit(1);
    }

    pid = fork();
    if(pid < 0){
        perror("fork()");
        exit(1);
    }

    if(pid == 0){
        close(pd[1]); // 关闭写端
        dup2(pd[0], 0);
        close(pd[0]);
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, 1);
        dup2(fd, 2);
        execl("/usr/local/bin/mpg123", "mpg123", "-", NULL);
        perror("execl()");
        exit(1);
    } else {
        close(pd[0]); // 关闭读端
        // 父进程从网上收数据往管道中写数据
        // ...
        close(pd[1]);
        wait(NULL);
        exit(0);
    }
}
