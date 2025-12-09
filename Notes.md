# 系统调用实现流程：以trace调用为例
在用户函数中trace()函数不能直接跳转内核，必须通过ecall陷入内核(RISC-V),并把系统调用号放入寄存器（SYS_trace）
ecall触发trap异常，CPU切换到内核态。

# 在用户态出现中断/异常流程
在启动时userinit()->allocproc()中设置forkret为当前进程上下文的地址，其中调用usertrapret()
### usertrapret流程：
这一步是从内核态切换回用户态，关键一步是关闭interrupt，因为在内核态时发生interrupt会触发的是kerneltrap
下一步是把stvec指向uservec
```C
w_stvec(TRAMPOLINE + (uservec - trampoline));
```
然后设置trapframe字段，如内核页表，内核栈，跳转函数，CPU id等，调用userret，内部检查为系统调用后，会执行syscall
### trace -> sys_trace?
```
p->trapframe->a0 = syscalls[num]();// num从a7读取，也就是trace的函数号，调用sys_trace函数，向当前进程植入trace_mask
```


# 在内核态中出现中断/异常流程
trapinithart在启动时写stvec这个寄存器(Supervisor Trap Vector Base Address register)，这个寄存器的作用是指定在发生trap时，CPU应当跳转到那个地址去执行处理程序（trap处理函数的入口），写为kernelvec

### kernelvec详细讲了trap发生的事情
首先是为保存寄存器分配栈空间，保存寄存器，call 中断处理函数，恢复寄存器，释放栈空间。栈空间释放本质就是移动栈指针，然后sret

### kerneltrap 过程
首先用SSTATUS_SPP这个bool变量记录当前是supervisor mode(1)还是user mode(0)
然后要求intr_get()为0，即中断必须关闭,保证trap handler原子执行
尝试处理设备终端和时钟中断