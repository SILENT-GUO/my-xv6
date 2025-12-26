#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define EXCHANGE_COUNT 1000000L
int main(int argc, char *argv[])
{
    int pipe_parent_child[2], pipe_child_parent[2];
    if (pipe(pipe_child_parent) == -1 || pipe(pipe_parent_child) == -1)
    {
        perror("Pipe Error.");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    char byte = 'A'; // 要交换的字节

    if (pid == 0)
    {
        // 子进程：接收 from parent, send to parent
        close(pipe_parent_child[1]); // 关闭写端
        close(pipe_child_parent[0]); // 关闭读端

        for (long i = 0; i < EXCHANGE_COUNT; i++)
        {
            if (read(pipe_parent_child[0], &byte, 1) != 1)
            {
                perror("child read");
                exit(EXIT_FAILURE);
            }
            if (write(pipe_child_parent[1], &byte, 1) != 1)
            {
                perror("child write");
                exit(EXIT_FAILURE);
            }
        }

        close(pipe_parent_child[0]);
        close(pipe_child_parent[1]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // 父进程：先发，再收，循环
        close(pipe_parent_child[0]); // 关闭读端
        close(pipe_child_parent[1]); // 关闭写端

        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (long i = 0; i < EXCHANGE_COUNT; i++)
        {
            if (write(pipe_parent_child[1], &byte, 1) != 1)
            {
                perror("parent write");
                exit(EXIT_FAILURE);
            }
            if (read(pipe_child_parent[0], &byte, 1) != 1)
            {
                perror("parent read");
                exit(EXIT_FAILURE);
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        close(pipe_parent_child[1]);
        close(pipe_child_parent[0]);

        // 等待子进程退出
        wait(NULL);
        double elapsed = (end.tv_sec - start.tv_sec) +
                         (end.tv_nsec - start.tv_nsec) / 1e9;

        double exchanges_per_sec = EXCHANGE_COUNT / elapsed;

        printf("Exchanges: %ld\n", EXCHANGE_COUNT);
        printf("Elapsed time: %.6f seconds\n", elapsed);
        printf("Performance: %.2f exchanges/second\n", exchanges_per_sec);
    }
}