#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    struct stat statres;
    char *s;
    int fd = open(argv[1], O_RDONLY);
    if(fd < 0){
        perror("open()");
        exit(1);
    }

    if(fstat(fd, &statres) < 0){
        perror("fstat()");
        exit(1);
    }

    s = (char*)mmap(NULL, statres.st_size, PROT_READ, MAP_SHARED, fd, SEEK_SET);
    if(s == MAP_FAILED){
        perror("mmap()");
        exit(1);
    }
    close(fd);
    int count = 0;
    for(int i = 0; i < statres.st_size; ++i){
        if(s[i] == 'a') count++;
    }
    printf("%d\n", count);
    munmap(s, statres.st_size);

    return 0;
}