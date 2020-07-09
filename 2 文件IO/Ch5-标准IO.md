# 标准IO


## 1 流和FILE

设置流的定向，不改变已定向流的定向
int fwide(FILE *stream, int mode);


## 2 缓冲

全缓冲：在填满标准IO缓冲区后才执行实际IO操作
	在流上执行第一次IO操作通常相关 函数调用malloc获得需要使用的缓冲区
	冲洗：fflush
行缓冲：
无缓冲

更改缓冲区类型，对一个打开的流未执行其他操作之前调用
	void setbuf(FILE *stream, char *buf);
	void setbuffer(FILE *stream, char *buf, size_t size);
	void setlinebuf(FILE *stream);
	int setvbuf(FILE *stream, char *buf, int mode, size_t size);

st_blksize -> 最佳IO缓冲区长度

fflush


## 3 打开流

	FILE *fopen(const char *path, const char *mode);
	FILE *fdopen(int fd, const char *mode);
	取一个已有的文件描述符，使一个标准IO流与该描述符相结合，常用于创建管道和网络通信通道函数返回的描述符
	FILE *freopen(const char *path, const char *mode, FILE *stream);

