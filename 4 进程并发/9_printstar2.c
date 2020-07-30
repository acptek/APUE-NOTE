#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static void int_handler(int s)
{
    write(1,"!",1);
}

int main()
{
    sigset_t set,oset,saveset;
    int i,j;

    signal(SIGINT,int_handler);

    sigemptyset(&set);
    sigaddset(&set,SIGINT);

    sigprocmask(SIG_UNBLOCK,&set,&saveset);

	// 阻塞信号集set中的信号
    sigprocmask(SIG_BLOCK,&set,&oset);
    for(j = 0 ; j < 10000; j++)
    {
        for(i = 0 ; i < 5; i++)
        {
            write(1,"*",1);
            sleep(1);
        }
        write(1,"\n",1);

        // 相当于下面三行的原子操作
        sigsuspend(&oset); // 恢复到oset状态，进入睡眠，收到信号恢复状态 的原子操作
    /*
        sigset_t tmpset;
        // 恢复原来状态，相当于解除阻塞，保存状态
        sigprocmask(SIG_SETMASK,&oset,&tmpset); // 因为信号驱动时，在该行执行完到pause()执行之前，可能发出的信号（或者之前保留等待执行的那一个信号）已经被响应了，没有响应到pause上，所以等待信号到来
        pause(); // 等待信号
        // 恢复状态，相当于恢复了阻塞
        sigprocmask(SIG_SETMASK,&tmpset,NULL);
    */
    }

    sigprocmask(SIG_SETMASK,&saveset,NULL);

    exit(0);
}
