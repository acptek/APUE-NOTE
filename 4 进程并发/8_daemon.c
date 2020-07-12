#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define FNAME "/tmp/out"

static int daemonize(void){
    int fd;
    pid_t pid;
    pid = fork();
    if(pid < 0){
        return -1;
    }

    if(pid > 0){
        exit(0);
    }

    fd = open("/dev/null", O_RDWR);
    if(fd < 0){
        return -1;
    }

    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    if(fd > 2)
        close(fd);

    setsid();

    chdir("/");
    // umask(0);

    return 0;

}

void tiktok(){
    int i;
    FILE *fp;

    fp = fopen(FNAME, "w");
    if(fp == NULL){
        syslog(LOG_ERR, "fopen fail : %s", strerror(errno));
        exit(1);
    }

    syslog(LOG_INFO, "%s is opened", FNAME);

    for(i = 0; ; ++i){
        fprintf(fp, "%d\n", i);
        fflush(fp);
        syslog(LOG_DEBUG, "%d is printed", i);
        sleep(1);
    }

    fclose(fp);
}

static void daemon_exit(int s){

    closelog();
    exit(0);
}

int main()
{

//    signal(SIGINT, daemon_exit);
//    signal(SIGQUIT, daemon_exit);
//    signal(SIGTERM, daemon_exit);
    struct sigaction sa;

    // 同时监听信号
    sa.sa_handler = daemon_exit;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGINT);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    openlog("daemon", LOG_PID, LOG_DAEMON);

    if(daemonize()){
        syslog(LOG_ERR, "daemonize() fail!"); // do not need '\n'
        exit(1);
    } else {
        syslog(LOG_INFO, "daemonize success!");
    }

    tiktok();


    exit(0);
}
