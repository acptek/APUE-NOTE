#include "mytbf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

typedef void (*sighandler_t)(int);

static struct mytbf_st * array[MYTBF_MAX];
static int inited = 0;
static sighandler_t alarm_handler_save;

struct mytbf_st{
    int cps;
    int burst;
    int token;
    int pos;
};

/**
 * SIGALRM信号处理函数
 * -------------------------------------------------
 * -> 继续发出alarm信号，构成alarm信号链
 * -> 每一个时钟时刻遍历所有的令牌桶，调整各自的状态
 */
static void alrm_handler(int s){
    int i;

    alarm(1);

    for(i = 0; i < MYTBF_MAX; ++i){
        if(array[i] != NULL){
            array[i]->token += array[i]->cps;
            if(array[i]->token > array[i]->burst){
                array[i]->token = array[i]->burst;
            }
        }
    }
}


/**
 * 模块卸载函数
 * -------------------------------------------------
 * -> 恢复SIGALRM信号处理入口
 * -> 立即关闭时钟
 * -> 释放令牌桶数组空间
 */
static void module_unload(void){
    int i;
    signal(SIGALRM, alarm_handler_save);
    alarm(0);

    for(i = 0; i < MYTBF_MAX; ++i)
        free(array[i]);
}

/**
 * 发出第一个信号的模块加载
 * -------------------------------------------------
 * -> 监听SIGALRM信号，启动初始信号
 * -> 保存信号发送前的函数入口
 */
static void module_load(void){
    alarm_handler_save = signal(SIGALRM, alrm_handler);
    alarm(1);

    atexit(module_unload);
}

/**
 * 初始化一个令牌桶返回一个mytbf_st结构体指针
 * -------------------------------------------------
 * 结构体指针保存在一个指针数组中，最多申请MYTBF_MAX个
 * -> 初始化的模块加载
 * -> inited设置为全局静态变量：所有令牌桶以相同的之中频率运转
 * -> 为分配的令牌桶分配存储位置
 * -> 初始化一个令牌桶的参数
 */
mytbf_t * mytbf_init(int cps, int burst){
    int i;
    struct mytbf_st *me;

    if(!inited) {
        module_load();
        inited = 1;
    }

    me = malloc(sizeof(struct mytbf_st));
    if(me == NULL)
        return NULL;

    // find the first pos which is not occupied
    for(i = 0; i < MYTBF_MAX; i++){
        if(array[i] == NULL){
            break;
        }
    }
    if(i >= MYTBF_MAX)
        return NULL;

    // init
    me->pos = i;
    me->cps = cps;
    me->burst = burst;
    me->token = 0;
    array[i] = me;

    return me;
}

/**
 * 获取令牌
 * @param tbf
 * @param size 要获取令牌的大小
 * @return 返回获取到的令牌的数量
 * -------------------------------------------------
 * 当token中没有值时，等待一个信号到来打断阻塞
 * 当一个SIGALRM信号到来会为token分配一定的值，此时可以返回值
 */
int mytbf_fetchtoken(mytbf_t *tbf, int size){
    int n;
    struct mytbf_st *me = tbf;
    if(size <= 0)
        return -EINVAL;

    while (me->token <= 0)
        pause();

    n = me->token < size ? me->token : size;

    me->token -= n;
    return n;
}

/**
 * 归还令牌
 * @param tbf
 * @param size
 * @return 返回大小为size的令牌数量
 * -------------------------------------------------
 * -> 直接将size的大小加到对应的token上
 */
int mytbf_returntoken(mytbf_t *tbf, int size){
    struct mytbf_st *me = tbf;
    if(size <= 0)
        return -EINVAL;

    me->token += size;
    if(me->token > me->burst)
        me->token = me->burst;

    return size;
}


/**
 * 销毁一个令牌桶
 * @param tbf
 * @return
 * 释放令牌桶指针
 */
int mytbf_destroy(mytbf_t *tbf){
    struct mytbf_st *me = tbf;
    array[me->pos] = NULL;
    free(tbf);

    return 0;
}
