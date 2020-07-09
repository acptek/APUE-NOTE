# 标准IO

```c
fopen, fclose

fgetc, getc, fputc
fgets, fputs

fread, fwrite

printf, fprintf sprintf snprintf  
scanf, fscanf sscanf
atoi

fseek, fseeko
ftell, ftello

fflush

getline

tmpnam, tmpfile
```



man手册 -> 标准库函数

stdio : 标准IO, FILE类型

## fopen
```
FILE *fopen(const char *path, const char *mode);

(errno : 一个全局变量，及时存放出错信息， perror，strerror. 在errno.h中定义了errno的值)
mode :
	r, r+, w, w+, a
	a+：以读和追加方式打开，文件位置指针初始化到文件首，但总写到EOF后
		
(函数返回值是指针变量，并且有逆操作 多半其返回值存放于堆上)
```

## fclose 



---

- （默认打开的流 stdin stout stderr
- （文件权限 ： 0666 & ~umask  -->  -rw-rw-r-- (664)



## fgetc
定义成函数（占用进程调用时间，不占用编译时间）

## getc
定义成宏（占用编译时间，不占用调用时间）

## fputc



---

## fgets

从输入流 stream 中读取一个字符串回填到 s 所指向的空间

正常结束两种可能：
- 到达 size-1
- '\n'

其他可能：（返回NULL）

- EOF
- 出错

## fputs



---
## fread
## fwrite



---

## printf : fprintf sprintf snprintf  
## scanf : fscanf sscanf

## atoi

---

## 文件指针
### fseek --> fseeko
	移动文件指针位置
	offset：基于 whence 参数的偏移量

- 空洞文件：fseek(fp, 2GB, SEEK_SET);

### ftell --> ftello
	反映文件指针当前位置，以字节为单位
	fseek(fp, 0, SEEK_END) + ftell(3) 可以计算出文件总字节大小。
	fseek返回值为long型，所以最大表示为2G-1字节，可以使用ftello替换
	
	ftello：方言（SUSv2, POSIX.1-2001.）

### rewind ==> fseek(stream, 0L, SEEK_SET)

---

### fflush
刷新一个指定的流



```	
缓冲区： 作用是 合并系统调用，
	行缓冲 ： 换行的时候刷新，满了的时候刷新，强制时候刷新（例如stdout：是终端设备
	全缓冲 ： 满了的时候刷新，强制刷新，（默认，只要不是终端设备）
	无缓冲 ： 例如stderr，这是需要立即输出的内容

修改缓冲区：setvbuf
```

---

## getline
```
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

方言，仅支持 GNU 标准。
在编译时：gcc -D_GNU_SOURCE 或者 在头文件之前加 #define _GNU_SOURCE
```
- TASK : 实现getline，以及进行内存释放



---

## 临时文件：
临时数据放在临时文件当中
-  不冲突
- 及时销毁
		
### tmpnam
为一个临时文件创建一个名字
### tmpfile
创建一个临时文件（匿名文件，不会产生名字冲突



