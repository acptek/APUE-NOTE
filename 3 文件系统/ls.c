#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <string.h>

#define PAT "./*"

static int errfunc(const char *errpath, int eerrno){
    puts(errpath);
    fprintf(stderr, "ERROR MSG: %s\n", strerror(eerrno));
    return 0;
}

int main(int argc, char **argv){
    glob_t globres;
    int err = glob(argv[1], 0, NULL, &globres);
    if(err) {
        printf("Error code = %d\n", err);
        exit(1);
    }

    size_t s = globres.gl_pathc;
    printf("len = %lu\n", s);

    for(int i = 0; i < globres.gl_pathc; ++i){
        puts(globres.gl_pathv[i]);
    }
//    int i = 0;
//    while (globres.gl_pathv[i] != NULL){
//        puts(globres.gl_pathv[i]);
//        ++i;
//    }
    globfree(&globres);
    exit(0);
}
