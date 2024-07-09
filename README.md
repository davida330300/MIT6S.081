# Lab 5

## part 1 

 We can see this
 ```
    hart 1 starting
    hart 2 starting
    init: starting sh
    $ echo hi
    usertrap(): unexpected scause 0x000000000000000f pid=3
                sepc=0x00000000000012a4 stval=0x0000000000004008
    $ 
 ```

 the scause number is 15, means load page fault, because of not mapping correctly

## part 2

* You can check whether a fault is a page fault by seeing if r_scause() is 13 or 15 in usertrap().
* r_stval() returns the RISC-V stval register, which contains the virtual address that caused the page fault.
* Steal code from uvmalloc() in vm.c, which is what sbrk() calls (via growproc()). You'll need to call kalloc() and mappages().
* Use PGROUNDDOWN(va) to round the faulting virtual address down to a page boundary.
* uvmunmap() will panic; modify it to not panic if some pages aren't mapped.
* If the kernel crashes, look up sepc in kernel/kernel.asm
* Use your vmprint function from pgtbl lab to print the content of a page table.
* If you see the error "incomplete type proc", include "spinlock.h" then "proc.h"

## part 3 
* Handle negative sbrk() arguments.
* Kill a process if it page-faults on a virtual memory address higher than any allocated with sbrk(). (OK)
* Handle the parent-to-child memory copy in fork() correctly.  
* Handle the case in which a process passes a valid address from sbrk() to a system call such as read or write, but the memory for that address has not yet been allocated.
* Handle out-of-memory correctly: if kalloc() fails in the page fault handler, kill the current process.
* Handle faults on the invalid page below the user stack.