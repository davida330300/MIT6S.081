# lab 4


## part 3

### test 0 : invoke handler
* Get started by modifying the kernel to jump to the alarm handler in user space, which will cause test0 to print "alarm!". Don't worry yet what happens after the "alarm!" output; it's OK for now if your program crashes after printing "alarm!". Here are some hints:

* You'll need to modify the Makefile to cause alarmtest.c to be compiled as an xv6 user program. The right declarations to put in user/user.h are:

```c++
    int sigalarm(int ticks, void (*handler)());
    int sigreturn(void);
```

* Update user/usys.pl (which generates user/usys.S), kernel/syscall.h, and kernel/syscall.c to allow alarmtest to invoke the sigalarm and sigreturn system calls. For now, your sys_sigreturn should just return zero.
* Your sys_sigalarm() should store the alarm interval and the pointer to the handler function in new fields in the proc structure (in kernel/proc.h).

`
    sigalarm read to argument, the period and alarm handler
`
```c++ 
    my_proc->alarm_period = period;
    my_proc->alarm_handler = (void (*)()) p;
    
```

* You'll need to keep track of how many ticks have passed since the last call (or are left until the next call) to a process's alarm handler; you'll need a new field in struct proc for this too. You can initialize proc fields in allocproc() in proc.c.

```c++
    my_proc->tick_from_last_alarm = 0;
```
* Every tick, the hardware clock forces an interrupt, which is handled in usertrap() in kernel/trap.c.
* You only want to manipulate a process's alarm ticks if there's a timer interrupt; you want something like
```c++
    if(which_dev == 2) {
        // whenever an interrupt, trigger tick_from_last_alarm +1
        p->tick_from_last_alarm += 1;
        // when the record reach period limit, exec handler
    }
```
* Only invoke the alarm function if the process has a timer outstanding. Note that the address of the user's alarm function might be 0 (e.g., in user/alarmtest.asm, periodic is at address 0).
* You'll need to modify usertrap() so that when a process's alarm interval expires, the user process executes the handler function. When a trap on the RISC-V returns to user space, what determines the instruction address at which user-space code resumes execution?
* It will be easier to look at traps with gdb if you tell qemu to use only one CPU, which you can do by running
    `make CPUS=1 qemu-gdb`
* You've succeeded if alarmtest prints "alarm!".

```
    $ make qemu
        xv6 kernel is booting

        hart 2 starting
        hart 1 starting
        init: starting sh
    $ ./alarmtest
        test0 start
        .............................................alarm!
        test0 passed
        test1 start
        ....alarm!
        ....alarm!
        ...alarm!
        ......alarm!
        ....alarm!
        ....alarm!
        ....alarm!
        ....alarm!
        ....alarm!
        .....alarm!
        test1 passed
        test2 start
        ............................................................alarm!
        test2 passed
```

### test 1/2: resume interrupted code

Chances are that alarmtest crashes in test0 or test1 after it prints "alarm!", or that alarmtest (eventually) prints "test1 failed", or that alarmtest exits without printing "test1 passed". 

```
    Why? Because we change the epc, the return address is a mess
```

To fix this, you must ensure that, when the alarm handler is done, control returns to the instruction at which the user program was originally interrupted by the timer interrupt. You must ensure that the register contents are restored to the values they held at the time of the interrupt, so that the user program can continue undisturbed after the alarm. Finally, you should "re-arm" the alarm counter after each time it goes off, so that the handler is called periodically.

As a starting point, we've made a design decision for you: user alarm handlers are required to call the sigreturn system call when they have finished. Have a look at periodic in alarmtest.c for an example. This means that you can add code to usertrap and sys_sigreturn that cooperate to cause the user process to resume properly after it has handled the alarm.

Some hints:

Your solution will require you to save and restore registers---what registers do you need to save and restore to resume the interrupted code correctly? (Hint: it will be many).
Have usertrap save enough state in struct proc when the timer goes off that sigreturn can correctly return to the interrupted user code.
Prevent re-entrant calls to the handler----if a handler hasn't returned yet, the kernel shouldn't call it again. test2 tests this.
Once you pass test0, test1, and test2 run usertests to make sure you didn't break any other parts of the kernel.