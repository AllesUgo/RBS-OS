#pragma once


void InitPhyPageTable(void);
void InitKernelAllIndexTable(void);

unsigned int AllocatePhyPage(void);
void FreePhyPage(unsigned int page);
int AllocatePage(unsigned int alloc_addr, unsigned int attribute);
int AllocatePageInOtherProcress(unsigned int alloc_addr,unsigned int attribute,unsigned int target_cr3);
void ResetTargetIndexTableItem(unsigned int cr3,unsigned int which) __attribute__((optimize("O0")));;
void ResetKernelUserSpace(void);

void FreePage(unsigned int page);

void *kmalloc(unsigned int size);
//将堆向上拓展size字节(向上对其到整数页)
void* heapup(unsigned int size);
//将堆向下缩小size字节(对齐到整数页，例如参数1字节也会实际缩小4096字节)
void heapdown(unsigned int size);

void kfree(void *ptr);

unsigned int CopyPageIndexTable(int is_clear_user_space);

void AsyncKernelPageIndexTable(unsigned int target_index_table_phy_addr);

int mmap(unsigned int v_addr,unsigned int phy_addr,unsigned int attribute);//将某个虚拟地址映射到指定物理地址，并标记该物理页被使用
void SignPhyPageUsed(unsigned int phy_addr);//标记某个物理页已经使用
unsigned int* GetKernelPageIndexTable(void);
void SetKernelPageIndexTable(unsigned int* index_table_v_addr);

void ResetUserNowPageIndexTable(void);
void AsyncNowKernelSpaceIndexTableToKernelSpace(void);
void AsyncKernelSpaceIndexTableToNowKernelSpace(void);

void* kheapup(unsigned int size);
void kheapdown(unsigned int size);

void kmalloc_init(unsigned int max_size);



