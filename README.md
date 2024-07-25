# Memory allocator 

To reduce the race condition in different cpu grabbing free page, we allocate free page for individual cpu. When a cpu is running out of free pages by itself, it can steal from other cpu's page

```c 
  // original 
  acquire(&kmem[cpu].lock);
  r = kmem[cpu].freelist;
  if (r) {
    kmem[cpu].freelist = r->next;
  }
  release(&kmem[cpu].lock);
  if (r)
```

1. For multiple CPU, be aware of interrupt and context swiching between CPUs, so we need to add push_off() to disable interrput

2. if r=kmem[cpu].freeelist = 0, means the all free page assigned to this cpu have been used => steal from other cpu

3. To avoid dead lock, we always steal in the same direction. n, n+1, n+2... n+N%N

4. When stealing, use lock to perserve atomic 

# buffer cache

The buffer cache lock need to fine grained (original is a global bcache.lock)
We decide to use multiple buckets, and a lock for each bucket. Remember, the bcache is global and universal to all cpu
```c++
int
hash(uint blockno)
{
  return blockno % BUCKETSIZE;
}
```

We can use ticks to replace bi-direction LRU cache, modified in buf.h & bio.c
```c++
  // struct buf *prev; // LRU cache list
  // struct buf *next;
  uchar data[BSIZE];
  uint lastuse;  // ticks
```

```c++
  for (int i = 0; i < BUCKETSIZE; i++) {
    initlock(&bcachebucket[i].lock, "bcachebucket");
    for (int j = 0; j < BUFFERSIZE; j++) {
      initsleeplock(&bcachebucket[i].buf[j].lock, "buffer");
    }
  }
```
initialize lock for every bucket, and 

Get id from hash, lcok it, iterates over the buffer cache in the bucket to check if the block is already cached. 
  If yes (b->dev == dev && b->blockno == blockno) 1. refcnt++, 2. update tick, 
  If no   allocate a free block from bucket.

Update dev, blockno and tick


bpin and bunpin are easy, change to hash and individual lock for every bucket

