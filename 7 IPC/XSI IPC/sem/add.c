#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <errno.h>

#define PROCNUM     20
#define FNAME       "/tmp/out"
#define LINESIZE    1024

int semid;

static void P(){
	// 对信号量数组中的元素做统一原子操作
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;
    while(semop(semid, &op, 1) < 0){
        if(errno != EINTR || errno != EAGAIN) {
            perror("semop()");
            exit(1);
        }
    }
}

static void V(){
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;
    op.sem_flg = 0;
    if(semop(semid, &op, 1) < 0){
        perror("semop()");
        exit(1);
    }
}

static void func_add(void){
    FILE *fp;
    int fd;
    char linebuf[LINESIZE];

    fp = fopen(FNAME, "r+");
    if(fp == NULL){
        perror("fopen()");
        exit(1);
    }

    // 从文件流里取一个
    fd = fileno(fp);

    P();
    fgets(linebuf, LINESIZE, fp);
    fseek(fp, 0, SEEK_SET);
    //sleep(1);
    fprintf(fp, "%d\n", atoi(linebuf)+1);
    fflush(fp);
    V();

    fclose(fp);
}

int main()
{
    pid_t pid;
    
    // 信号量数组元素个数为1 // 使用IPC_PRIVATE代替使用ftok()
    semid = semget(IPC_PRIVATE, 1, 0600);
    if(semid < 0){
        perror("semget()");
        exit(1);
    }
    
    // 设置信号量的值为1
    if(semctl(semid, 0, SETVAL, 1) < 0){
        perror("semctl()");
        exit(1);
    }

    for (int i = 0; i < PROCNUM; ++i) {
        pid = fork();
        if(pid < 0){
            perror("fork()");
            exit(1);
        }
        if(pid == 0){
            func_add();
            exit(0);
        }
    }
    for (int i = 0; i < PROCNUM; ++i) {
        wait(NULL);
    }

    semctl(semid, 0, IPC_RMID);

    exit(0);
}
