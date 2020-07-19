#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

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
    
    // 创建一个子进程读父进程写的管道
    
    if(pid == 0){
        close(pd[1]);
        len = read(pd[0], buf, BUFSIZE); // 从管道中读数据, 如果管道中没有数据会阻塞
        write(1, buf, len);
        close(pd[0]);
        exit(0);
    } else {
    	sleep(1);
        close(pd[0]);
        write(pd[1], "Hello!", 6);
        close(pd[1]);
        wait(NULL);
        exit(0);
    }
}
