#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define     COUNT  10
int main()
{
    char buf[COUNT];
    size_t nbytes;
    int n, flags;
    int fd;

    for (n = 0; n < COUNT; n++)
        buf[n] = 0;

    //flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    //flags |= O_NONBLOCK;
    //fcntl(STDIN_FILENO, F_SETFL, flags);

    fd = open("/dev/tty11", O_RDONLY|O_NONBLOCK);
    printf("%d\n", fd);
    nbytes = 10;
    int rdnum = read(fd, buf, nbytes);
    if(rdnum < 0){
        fprintf(stdout, "%s\n", strerror(errno));
    }
    printf("%d\n", rdnum);
    close(fd);

    printf("hello\n");
    printf("%s\n", buf);
}
