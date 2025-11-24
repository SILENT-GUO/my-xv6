#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int parent_child_pipe[2], child_parent_pipe[2];

    char byte = 'a';

    if(pipe(parent_child_pipe) < 0 || pipe(child_parent_pipe) < 0){
        fprintf(2, "Pipe failed.\n");
        exit(1);
    }

    if(fork() == 0){
        // child process
        close(parent_child_pipe[1]);
        close(child_parent_pipe[0]);
        if(read(parent_child_pipe[0], &byte, 1) != 1){
            fprintf(2, "Child: Read from pipe failed.\n");
            exit(1);
        }

        printf("%d: received ping\n", getpid());
        close(parent_child_pipe[0]);

        if(write(child_parent_pipe[1], &byte, 1) != 1){
            fprintf(2, "Child: Write into pipe failed.\n");
            exit(1);
        }
        close(child_parent_pipe[1]);
        
        exit(0);
    }else{
        close(parent_child_pipe[0]);
        close(child_parent_pipe[1]);
        if(write(parent_child_pipe[1], &byte, 1) != 1){
            fprintf(2, "Parent: Write into pipe failed.\n");
            exit(1);
        }
        close(parent_child_pipe[1]);
        if(read(child_parent_pipe[0], &byte, 1) != 1){
            fprintf(2, "Parent: Read froom pipe failed.\n");
            exit(1);
        }
        close(child_parent_pipe[0]);
        printf("%d: received pong\n", getpid());

        wait(0);
        exit(0);
    }
}