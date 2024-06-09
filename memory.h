#pragma once


void InitPhyPageTable(void);
void InitKernelAllIndexTable(void);

unsigned int AllocatePhyPage(void);
void FreePhyPage(unsigned int page);
int AllocatePage(unsigned int alloc_addr, unsigned int attribute);

void FreePage(unsigned int page);

void *kmalloc(unsigned int size);

void kfree(void *ptr);

unsigned int CopyPageIndexTable(void);

void AsyncKernelPageIndexTable(unsigned int target_index_table_phy_addr);

int mmap(unsigned int v_addr,unsigned int phy_addr,unsigned int attribute);//将某个虚拟地址映射到指定物理地址，并标记该物理页被使用
void SignPhyPageUsed(unsigned int phy_addr);//标记某个物理页已经使用
unsigned int* GetKernelPageIndexTable(void);
void SetKernelPageIndexTable(unsigned int* index_table_v_addr);

void ResetUserNowPageIndexTable(void);
void AsyncNowKernelSpaceIndexTableToKernelSpace(void);
void AsyncKernelSpaceIndexTableToNowKernelSpace(void);
