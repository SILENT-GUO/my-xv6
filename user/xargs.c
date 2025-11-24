#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

int readline(char *line, int max_len){
    char c;
    int i = 0;
    while(i < max_len - 1){
        int n = read(0, &c, 1);
        if(n == 0){
            if(i == 0){
                return 0;
            }else{
                break;
            }
        }
        if(c == '\n'){
            break;
        }
        line[i++] = c;
    }
    line[i] = '\0';
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "xargs args ...\n");
        exit(1);
    }
    char line[512];
    while(readline(line, sizeof(line))){
        int pid = fork();
        if(pid < 0){
            fprintf(2, "fork failed.\n");
            exit(1);
        }
        if(pid == 0){
            //child process;
            char *args[MAXARG];
            int i = 0;

            for(i = 1; i < argc && i < MAXARG; ++i){
                args[i-1] = argv[i];
            }
            if(i < MAXARG - 1){
                args[i-1] = line;
                args[i] = 0;
            }else{
                fprintf(2, "too much args.\n");
                exit(1);
            }
            exec(args[0], args);
            fprintf(2, "xargs: exec failed.\n");
            exit(1);
        }
        else{
            wait((int *)0);
        }
    }
    exit(0);
}