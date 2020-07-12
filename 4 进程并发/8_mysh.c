#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <glob.h>

#define N 1000

struct command{
    glob_t globres;
};

static void perm() {
    printf("$ ");
}

static void analy(char *str, struct command *cmd){
    const char *delim = " \t\n";
    char *token;
    int globnum;
    int flag = 0;

    while(1){
        token = strsep(&str, delim);
        if(token == NULL)
            break;
        if(token[0] == '\0')
            continue;
        // flag 用于初始化：第一次不追加内容到globres之后都追加
        // GLOB_NOCHECK：表示没有匹配token就直接返回
        glob(token, GLOB_NOCHECK|GLOB_APPEND*flag, NULL, &cmd->globres);
        flag = 1;
    }

//    for(token =token strsep(&str, delim); token != NULL; token = strsep(&str, delim)){
//        globnum = glob();
//    }
}

int main(){
    pid_t pid;
    size_t len = 0;
    char *line = NULL;
    struct command cmd;
    while(1){
        perm();

        if(getline(&line, &len, stdin) < 0){
            printf("getline() Error\n");
            break;
        }

        analy(line, &cmd);

        if(0){
            // 内部命令
        } else {
            pid = fork();
            if(pid < 0){
                perror("fork()");
                exit(1);
            }
            if(pid == 0){
//                for(int i = 0; i < cmd.globres.gl_pathc; ++i){
//                    printf("%s\n", cmd.globres.gl_pathv[i]);
//                } printf("\n");
                fflush(stdout);
                execvp(cmd.globres.gl_pathv[0], cmd.globres.gl_pathv);
                perror("execvp() Error!");
                exit(1);
            } else {
                wait(NULL);
            }
        }
    }
    exit(0);
}
