
# 登录

```
shan:x:1000:1000:Shan,,,:/home/shan:/bin/bash
```
登录名，加密口令，UID，GID，注释字段，起始目录，shell程序(可以指定解释器

执行shell文件命令：
	chmod +x ./[name].sh :添加可执行权限
	./[name].sh


# 文件和目录

shell的引号机制：
	单引号：作为普通字符
	反单引号：命令替换
		反单引号的内容（先被执行）的结果替换原来位置的内容 ls `pwd`
	双引号：普通字符，定义变量，变量替换和命令替换 "$"

工作目录
	chdir()


# 输入和输出
	文件描述符
	标准输入输出错误
	不带缓冲的IO
		shell ： ./a.out < in > out : 将in中的内容复制到out中
	标准IO


# 程序和进程
	程序
	进程 & 进程ID
	线程 & 线程ID
	
	命令行管道 |


# 信号



# 进程 STAT

	D    不可中断     Uninterruptible sleep (usually IO)
	R    正在运行，或在队列中的进程
	S    处于休眠状态
	T    停止或被追踪
	Z    僵尸进程
	W    进入内存交换（从内核2.6开始无效）
	B    进程正在等待更多的内存页
	I    空闲状态
	O    进程正在处理器运行
	X    死掉的进程


    <    高优先级
    N    低优先级
    L    有些页被锁进内存
    s    包含子进程
    +    位于后台的进程组；
    l    多线程，克隆线程  multi-threaded (using CLONE_THREAD, like NPTL pthreads do)


-用户-组-其他用户
“-”常规文件
“d”目录
“l”符号链接
“c”字符专门设备文件
“b”块专门设备文件
“p”先进先出
“s”套接字



# 时间值

进程执行时间
	始终时间
	用户CPU时间
	系统CPU时间

