#include "kernel/types.h"
#include "user/user.h"

void seive(int fd);

int main(int argc, char *argv[]){
    // up to 35
    int prime_pipe[2];
    if(pipe(prime_pipe) < 0){
        fprintf(2, "Prime pipe failed.\n");
        exit(1);
    }
    if(fork() == 0){
        close(prime_pipe[1]);
        seive(prime_pipe[0]);
        exit(0);
    }else{
        close(prime_pipe[0]);
        for(int i = 2; i <= 35; ++i){
            if(write(prime_pipe[1], &i, sizeof(int)) != sizeof(int)){
                fprintf(2, "Write from main process failed.\n");
                break;
            }
        }
        close(prime_pipe[1]);

        wait(0);

        exit(0);
    }
}

void seive(int fd){
    int num;
    int prime;

    int p[2];

    if(read(fd, &prime, sizeof(int)) == sizeof(int)){
        printf("prime %d\n", prime);
        if(pipe(p) < 0){
            fprintf(2, "Seive pipe failed.\n");
            exit(1);
        }
        if(fork() == 0){
            close(p[1]);
            seive(p[0]);

            exit(0);
        }
        else{
            close(p[0]);
            while(read(fd, &num, sizeof(int)) == sizeof(int)){
                if(num % prime != 0){
                    if(write(p[1], &num, sizeof(int)) != sizeof(int)){
                        fprintf(2, "Write to pipe failed. \n");
                        break;
                    }
                }
            }
            close(p[1]);
            close(fd);
            wait(0);
        }

    }
    close(fd);
}