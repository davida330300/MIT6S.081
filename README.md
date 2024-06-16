# lab3 note

## print pg table

### Internal Node
  PTE_V(enabled)

### Leaf Node
  PTE_W PTE_X PTE_R PTE_U(enabled)

## per proc kernel page
steps

1. Add a field to struct proc for the process's kernel page table.
```c
// kernel/proc.h
struct proc {
  pagetable_t k_pgtbl;      // per proc kernel page
};
```
2. A reasonable way to produce a kernel page table for a new process is to implement a modified version of kvminit that makes a new page table instead of modifying kernel_pagetable. You'll want to call this function from allocproc.
```c
// kernel/vm.c
// kvmmap, but for per proc kernel 
void
ukvmmap(pagetable_t k_pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
  if(mappages(k_pgtbl, va, sz, pa, perm) != 0) {
    panic("ukvmap");
  }
}

pagetable_t 
per_proc_kvminit() {
  // create an empty user page table.
  pagetable_t k_pgtbl = uvmcreate();
  if(k_pgtbl == 0) return 0;

  // instead of using global kernel page, we create k_pgtbl per proc and use this to init
  // others are similiar to kvminit()
  ukvmmap(k_pgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W);    // UART register
  ukvmmap(k_pgtbl, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);    // mmio disk interface
  ukvmmap(k_pgtbl, CLINT, CLINT, 0x10000, PTE_R | PTE_W);  
  ukvmmap(k_pgtbl, PLIC, PLIC, 0x400000, PTE_R | PTE_W);
  ukvmmap(k_pgtbl, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);
  ukvmmap(k_pgtbl, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);
  ukvmmap(k_pgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);  // high address of kernel
  return k_pgtbl;

}

```
3. Make sure that each process's kernel page table has a mapping for that process's kernel stack. In unmodified xv6, all the kernel stacks are set up in procinit. You will need to move some or all of this functionality to allocproc.

```c
  // kernel/proc.c allocproc(void):
  // allocate a per proc kernel pgtbl
  p->k_pgtbl = per_proc_kvminit();
  if(p->k_pgtbl == 0) {
    freeproc(p);
    release(&p->lock);
    return 0; 
  }

  // remap the kernel stack page per process
  // physical address is already allocated in procinit()
  // 
  uint64 va = KSTACK((int) (p - proc));
  pte_t pa = kvmpa(va);
  memset((void *)pa, 0, PGSIZE);
  ukvmmap(p->k_pgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  p->kstack = va;

```

4. Modify scheduler() to load the process's kernel page table into the core's satp register (see kvminithart for inspiration). Don't forget to call sfence_vma() after calling w_satp().
scheduler() should use kernel_pagetable when no process is running.

SATP : 

```c
// void scheduler(void)
// When context switching, 
  w_satp(MAKE_SATP(p->k_pgtbl));  // Load SATP (Supervisor Address Translation and Protection) register with the page table of the current process.
  sfence_vma(); //  flushes the TLB, ensuring that subsequent memory accesses use the new page table entries.

  swtch(&c->context, &p->context);

  // switch back to global kernel page
  w_satp(MAKE_SATP(kernel_pagetable));
  sfence_vma();

```
5. Free a process's kernel page table in freeproc.

```c
// kernel/proc.c static void freeproc(struct proc *p)
static void
freeproc(struct proc *p)
{
  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;

  if (p->k_pgtbl) {
    freeprockvm(p);
    p->k_pgtbl = 0;
  }
  if (p->kstack) {
    p->kstack = 0;
  }
}
```
6. You'll need a way to free a page table without also freeing the leaf physical memory pages.
vmprint may come in handy to debug page tables.

```c
void
ukvmunmap(pagetable_t pagetable, uint64 va, uint64 npages)
{
  uint64 a;
  pte_t *pte;

  if((va % PGSIZE) != 0)
    panic("ukvmunmap: not aligned");

  for(a = va; a < va + npages*PGSIZE; a += PGSIZE) {
    // clean page one by one
    if((pte = walk(pagetable, a, 0)) == 0)
      goto clean;
    if((*pte & PTE_V) == 0)
      goto clean;
    if(PTE_FLAGS(*pte) == PTE_V)
      panic("ukvmunmap: not a leaf");

    clean:
      *pte = 0;
  }
}

void freeprockvm(struct proc* p) {
  pagetable_t k_pgtbl = p->k_pgtbl;
  // unmap in reverse order from alloc, size / size of page
  ukvmunmap(k_pgtbl, p->kstack, PGSIZE/PGSIZE);
  ukvmunmap(k_pgtbl, TRAMPOLINE, PGSIZE/PGSIZE);
  ukvmunmap(k_pgtbl, (uint64)etext, (PHYSTOP-(uint64)etext)/PGSIZE);
  ukvmunmap(k_pgtbl, KERNBASE, ((uint64)etext-KERNBASE)/PGSIZE);
  ukvmunmap(k_pgtbl, PLIC, 0x400000/PGSIZE);
  ukvmunmap(k_pgtbl, CLINT, 0x10000/PGSIZE);
  ukvmunmap(k_pgtbl, VIRTIO0, PGSIZE/PGSIZE);
  ukvmunmap(k_pgtbl, UART0, PGSIZE/PGSIZE);
  // ref : uvmfree()
  freewalk(k_pgtbl);
}
```

7. It's OK to modify xv6 functions or add new functions; you'll probably need to do this in at least kernel/vm.c and kernel/proc.c. (But, don't modify kernel/vmcopyin.c, kernel/stats.c, user/usertests.c, and user/stats.c.)
8. A missing page table mapping will likely cause the kernel to encounter a page fault. It will print an error that includes sepc=0x00000000XXXXXXXX. You can find out where the fault occurred by searching for XXXXXXXX in kernel/kernel.asm.