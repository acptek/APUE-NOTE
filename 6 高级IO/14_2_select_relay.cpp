//
// Created by acptek on 2020/3/23.
//

#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#define TTY1 "/dev/tty11"
#define TTY2 "/dev/tty12"

#define BUFSIZE 4096

enum
{
    STATE_R=1,
    STATE_W,
    STATE_AUTO, // 基准线，下面是无条件推动状态机
    STATE_Ex,
    STATE_T
};

struct fsm_st{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len; // buf缓冲区中的字节长度
    int pos; // 缓冲区中还未写的数据的起始坐标
    char *err;
};

static void fsm_driver(struct fsm_st *fsm)
{
    int ret;
    switch (fsm->state){
        case STATE_R:
            fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
            if(fsm->len == 0)
                fsm->state = STATE_T;
            else if (fsm->len < 0){
                if(errno == EAGAIN)     // 连续做read操作而没有数据可读，非阻塞中read返回一个错误EAGAIN （由于IO设备占用而造成了没有数据可读）
                    fsm->state = STATE_R;
                else {
                    fsm->err = "read()";
                    fsm->state = STATE_Ex;
                }
            } else {        // 这里 fsm->len > 0 ， 若 fsm->fsd 的长度为 2×BUFFSIZE
                fsm->state = STATE_W;
                fsm->pos = 0;
            }
            break;

        case STATE_W:
            ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
            if(ret < 0){
                if(errno == EAGAIN)
                    fsm->state = STATE_W;
                else {
                    fsm->err = "write()";
                    fsm->state = STATE_Ex;
                }
            } else{ // 坚持写len个字节
                fsm->pos += ret;
                fsm->len -= ret;
                if(fsm->len == 0)
                    fsm->state = STATE_R;
                else
                    fsm->state = STATE_W;
            }
            break;

        case STATE_Ex:
            perror(fsm->err);
            fsm->state = STATE_T;
            break;

        case STATE_T:
            /* do sth. */
            break;

        default:
            abort(); // 出错人为制造一个异常得到文件
            break;
    }
}

void relay(int fd1, int fd2){
    fd_set rset, wset;
    int fd1_save, fd2_save;
    struct fsm_st fsm12, fsm21;
    fd1_save = fcntl(fd1, F_GETFL); // 获取文件状态
    fcntl(fd1, F_SETFL, fd1_save|O_NONBLOCK); // 设置文件属性：原有属性+nonblock属性

    fd2_save = fcntl(fd2, F_GETFL); // 获取文件状态
    fcntl(fd2, F_SETFL, fd2_save|O_NONBLOCK);

    // init
    fsm12.state =STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;
    fsm21.state =STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;

    while (fsm12.state != STATE_T || fsm21.state != STATE_T){

        // 布置监视任务
        FD_ZERO(&rset);
        FD_ZERO(&wset);

        if(fsm12.state == STATE_R){
            FD_SET(fsm12.sfd, &rset);
        }
        if(fsm12.state == STATE_W){
            FD_SET(fsm12.dfd, &wset);
        }
        if(fsm21.state == STATE_R){
            FD_SET(fsm21.sfd, &rset);
        }
        if(fsm21.state == STATE_W){
            FD_SET(fsm21.dfd, &wset);
        }

        // 监视 (当文件描述符发生相应行为，select返回，然后再进行有目的的操作)
        if(fsm12.state < STATE_AUTO || fsm21.state < STATE_AUTO) {

            if (select(std::max(fd1, fd2) + 1, &rset, &wset, NULL, NULL) < 0) {
                if (errno == EINTR)
                    continue;
                perror("select()");
                exit(1);
            }
        }

        // 查看监视结果
        // 根据监视结果来推动状态机
        if(FD_ISSET(fd1, &rset) || FD_ISSET(fd2, &wset) || fsm12.state > STATE_AUTO) {
            fsm_driver(&fsm12);
        }
        if(FD_ISSET(fd2, &rset) || FD_ISSET(fd1, &wset) || fsm21.state > STATE_AUTO) {
            fsm_driver(&fsm21);
        }
    }

    fcntl(fd1, F_SETFL, fd1_save|O_NONBLOCK);
    fcntl(fd2, F_SETFL, fd2_save|O_NONBLOCK);

}

int main() {

    int fd1, fd2;
    fd1 = open(TTY1, O_RDWR);
    if(fd1 < 0){
        perror("open()");
        exit(1);
    }

    fd2 = open(TTY2, O_RDWR|O_NONBLOCK);
    if(fd1 < 0) {
        perror("open()");
        exit(1);
    }

    relay(fd1, fd2);

    close(fd1);
    close(fd2);
    return 0;
}

/*
 * 1 关于EAGAIN的说明
 * 如果一个数据中继正在进行，此时正在从2设备上向1设备上写数据，这时数据中继希望从1设备上请求读数据写入设备2。
 * 正常情况下，设备1进行阻塞读操作，现在设置为非阻塞模式，所以表示当前无法从设备1上读数据返回错误EAGAIN。
 *
 * 2 关于读写的说明
 * 打开的文件描述符表示一个设备
 * 最开始两个设备都为空，此时大家都停留在read态
 *
 * 3 对同一个fd 进行 read 和 write 操作，都会改变lseek_cur的当前位置
 *      因此，还是对设备、管道等进行操作有意义
 */