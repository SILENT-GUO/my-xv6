# 操作系统接口
### 进程
每个程序都有包含的指令，数据和栈空间。指令：实现程序运算，数据为过程的变量，栈空间决定过程的调用。系统调用在一个进程需要调用内核服务时调用，方式是让进程陷入内核，执行服务然后返回。
### 内核
传统内核用**CPU硬件机制**保护实现在用户状态下确保进程只能返回自己的内存。
1. 特权级：现代CPU一般有Ring 0 - Ring 3，Ring 0为最高特权级，Ring 3为最低特权级，CPU在执行特权指令时会自动检测当前特权级，如果不符（用户程序执行特权指令则出触发异常），内核捕获后通常终止该进程
2. 内存保护：每个进程都有独立的虚拟地址空间，CPU的MMU根据页表完成虚拟地址到物理地址的转换 (具体过程参考lab pgtbl)
这些机制是硬件/电路层面的实现，不依赖软件
### 常见的系统调用接口
1. fork:
```C
int* pid = fork();
if (pid > 0)
{
   printf("This is parent process: child process pid =%d\n", pid);
   pid = wait((*int* *)0);
}
else if (pid == 0)
{
   printf("child: exiting\n");
   exit(0);
}
else
{
   printf("fork error\n");
}
```
其中exit系统调用会将调用的进程退出，释放内存和打开的文件（如文件描述符，告知进程不需要引用内核中的打开文件表项）等，参数一般0为成功，1-255失败

2. exec系统调用：如execve，覆盖原进程的代码段，设置为新ELF头指定的入口地址（见pgtbl lab详解）

3. I/O接口: read / write: ```read(fd, buf, n)```是从fd中读取不超过n个直接的数据并复制到buffer中，并返回读取的字节数。值得注意的是，fd会在fork和exec执行时得到保留，以此来实现IO重定向。每个底层文件的偏移量是父子共享的，如
```C
 if (fork() == 0)
 {
     write(1, "hello ", 6);
     exit(0);
 }
else
{
    wait(0);
    write(1, "world\n", 6);
}
```
这段代码的输出是 hello world\n。dup接口来复用时会指向同一个底层I/O对象

4. pipe管道。本身是一个小的内核缓冲区
标准方法实现管道
```C
int p[2];
char *argv[2];
argv[0] = "wc";
argv[1] = 0;
pipe(p);
if (fork() == 0)
{
    close(0);  // 释放文件描述符0
    dup(p[0]); // 复制一个p[0](管道读端)，此时文件描述符0（标准输入）也引用管道读端，故改变了标准输入。
    close(p[0]);
    close(p[1]);
    exec("/bin/wc", argv); // wc 默认从标准输入读取数据，并写入到参数中的每/一个文件
    // 因此我们需要改变默认输入为p[0]
}
else
{
    close(p[0]);
    write(p[1], "hello world\n", 12);
    close(p[1]);
}
```

5. 文件系统
记录一些常用的系统调用方法。如chdir("/a")可以修改当前进程的当前目录。
文件名是对文件的引用方式，真正的文件内容和属性存储在inode中，而硬链接 (ln)可以将多个文件名指向同一个inode，增加inode的链接时link count + 1，只有它降为0时，文件系统才会释放inode和数据块

1.6练习1
代码见pipe_bench.c
编译方法： gcc -O2 -o pipe_bench pipe_bench.c -lrt
ECS实例（2核2G性能）:
![alt text](image.png)
本地测试待完成
ECS实例性能低于预期一些原因：
通过cat /proc/cpuinfo观察CPU情况，发现实际只有一个物理核心，2核是两个vcpu，因此子进程和父进程之间会发生大量的切换