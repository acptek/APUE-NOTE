//
// Created by acptek on 2020/3/31.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <wait.h>

#define MEMSIZE 1024

int main()
{
    char *ptr;
    ptr = (char*)mmap(NULL, MEMSIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(ptr == MAP_FAILED){
        perror("mmap()");
        exit(1);
    }

    pid_t pid = fork();
    if(pid < 0){
        perror("fork()");
        munmap(ptr, MEMSIZE);
        exit(1);
    }

    if(pid == 0){
        strcpy(ptr, "Hello!");
        munmap(ptr, MEMSIZE);
        exit(0);
    } else{
        wait(NULL);
        puts(ptr);
        munmap(ptr, MEMSIZE);
        exit(0);
    }

    return 0;
}

/*
 * 父子进程通信
 * 共享内存
 */