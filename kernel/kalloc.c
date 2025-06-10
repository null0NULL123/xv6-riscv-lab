// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

// 内核在内存中的结束位置，即可用内存的开始位置
extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;

  // 页面对齐，向上舍入到最接近的页边界
  p = (char*)PGROUNDUP((uint64)pa_start);

  // 循环遍历每个物理页
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  // 1. 检查地址是否按页对齐（必须是 4096 字节的整数倍）
  // 2. 确保地址不在内核代码和数据区域内（end 是内核结束后的第一个地址）
  // 3. 验证地址不超过物理内存上限（PHYSTOP）
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  // 访问已经释放的内存就会读取到0x01的值
  memset(pa, 1, PGSIZE);

  // 把物理页的起始地址 pa 强制类型转换为 struct run* 类型
  // 这样做的目的是把这块物理内存的头部用来存储链表信息
  r = (struct run*)pa;

  // 将一块物理内存页插入到空闲链表头部
  // 方便后续的内存分配时直接从链表头部取出
  // 因此链表的顺序是后进先出
  // 由于freelist 初始值是 NULL
  // 所以链表的最后一个元素的 next 指针是 NULL
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  // 取出链表头部的空闲页
  r = kmem.freelist;
  // 如果有空闲页，把链表头后移，将分配出去的页从链表中移除
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  // 当没有可用的物理页时，返回 NULL，即 0
  return (void*)r;
}
