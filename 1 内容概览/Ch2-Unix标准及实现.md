# UNIX标准及实现

## 1 UNIX标准化

	1  ISO C
		ISO/IEC 9899:1999
	2  IEEE POSIX
		可移植操作系统接口
		ISO/IEC 9945-1:1990 <==> POSIX.1
	  ->IOS/IEC 9945-1:1996 -> ... -> IOS/IEC 9945:2009
	3  SUS

## 2 限制

	1  ISO C
		-> <limits.h>
	2  POSIX
		数值限制，最小值，最大值，运行时可以增加的值，
		运行时不变的值，其他不变值，路径名可变值
	3  XSI
	4  函数
		sysconf , pathconf , fpathconf


