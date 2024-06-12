#include "memory.h"
#include "base_io.h"
#include "task_basic_info.h"
#include "base_type.h"
#include "screen_ctrl.h"
#include "task.h"
#include "basic_info.h"

#define PALIGN_UP(x, _al) ((x + (_al - 1)) & ~(_al - 1))
static unsigned int *KERNEL_PAGE_INDEX_TABLE_ADDR;
static unsigned int NEXT_ALLOCATE = 0x80202000;
static struct KERNEL_MALLOC_INFO {
    unsigned int start_vaddr;
    unsigned int map_size_byte;
    char* map;
} KMALLOC_INFO={0,0,0};

void InitPhyPageTable(void)
{
    unsigned int *table = (unsigned int *)PHY_PAGE_TABLE_ADDR;
    for (int i = 0; i < 16; ++i)
        table[i] = 0xFFFFFFFF; // 前2M，即前512页已经使用
}

void InitKernelAllIndexTable(void)
{
    unsigned int *k = (unsigned int *)0xFFFFF000;
    for (int i = 512; i < 1023; ++i)
    {
        if ((k[i] & 1) == 0)
        {
            k[i] = AllocatePhyPage();
            k[i] |=0x03;
            unsigned int* pt = (unsigned int*)(0xFFC00000|(i<<12));
            for (int j = 0;j<1024;++j)
            {
                pt[j] = 0;
            }
        }
    }
}

unsigned int AllocatePhyPage(void)
{
    unsigned int *table = (unsigned int *)PHY_PAGE_TABLE_ADDR;
    for (unsigned int i = 0; i < 32768; ++i)
    {
        unsigned int temp = table[i];
        for (unsigned int k = 0; k < 32; ++k)
        {
            if ((temp & 0x80000000) == 0)
            {
                // 这个地址可用
                table[i] |= (1 << (32 - 1 - k));
                return (i * 32 + k) << 12;
            }
            temp <<= 1;
        }
    }
    return 0xFFFFFFFF;
}
void FreePhyPage(unsigned int page)
{
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    unsigned int *table = (unsigned int *)PHY_PAGE_TABLE_ADDR;
    page >>= 12;
    unsigned int n = page / 32;
    int a = page % 32;
    table[n] &= ~(1 << (32 - a - 1));
    SetEFLAGS(eflags);
}
int AllocatePage(unsigned int alloc_addr, unsigned int attribute)
{
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    /*
    {
        char temp[] = "allocate";
        Puts(temp);
        PrintHex(alloc_addr);
    }
    */
    // 先检查要申请的内存在页目录表中是否存在
    unsigned int temp = alloc_addr >> 22;
    temp <<= 2;
    temp |= 0xFFFFF000;

    // PrintHex(temp);

    if (((*(unsigned int *)temp) & 1) == 0)
    {
        /*
        {
            char str[] = "Not found in page index table\r\n";
            Puts(str);
        }
        */
        // 不在内存中
        unsigned int page_table = AllocatePhyPage(); // 申请物理页
        if (page_table == 0xFFFFFFFF)
        {
            SetEFLAGS(eflags);
            return -1; // 申请物理页失败
        }
        if (alloc_addr >= 0x80000000)
            *(unsigned int *)temp = page_table | 0x03;//内核页目录权限限制
        else
            *(unsigned int *)temp = page_table | 0x07; // 在页目录中注册该页表
        temp = alloc_addr & 0xFFC00000;
        temp >>= 10;
        temp |= 0xFFC00000;
        // 将页表清空
        for (int i = 0; i < 1024; ++i)
            *((unsigned int *)temp + i) = 0;
        // 在最后一项填写自身地址
        //*((unsigned int *)temp + 1023) = (page_table & 0xFFFFF000) | 0x03;
    }
    // 检查页是否在页表中
    temp = 0xFFC00000;
    temp |= (alloc_addr >> 12) << 2;
    // PrintHex(temp);

    if (((*(unsigned int *)temp) & 1) == 0)
    {
        /*
        {
            char str[] = "Not found in page table\r\n";
            Puts(str);
        }
        */
        // 页表中没有该物理页
        unsigned int page = AllocatePhyPage();
        if (page == 0xFFFFFFFF)
        {
            SetEFLAGS(eflags);
            return -1; // 申请物理页失败
        }
        *(unsigned int *)temp = page | attribute; // 普通页无需初始化
    }
    asm volatile(
        "push %%eax\n\t"
        "mov %%cr3,%%eax\n\t"
        "mov %%eax,%%cr3\n\t"
        "pop %%eax\n\t"
        :
        :);
    /*
    {
        char str[] = "allocate finish\r\n";
        Puts(str);
    }
    */
    SetEFLAGS(eflags);
    return 0; // 分配成功
}

int AllocatePageInOtherProcress(unsigned int alloc_addr,unsigned int attribute,unsigned int target_cr3)
{
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    //先将目标cr3载入当前cr3的倒数第二项
    unsigned int temp = 0xFFFFF000 + 1022 * 4; // 指向当前页目录到数第二项
    *(unsigned int *)temp = target_cr3 | 0x03;
    asm volatile(
        "invlpg (0xFFFFFFF8)\n\t"
        :
        :); // 刷新页目录表里该条目
    /*
    {
        char temp[] = "allocate";
        Puts(temp);
        PrintHex(alloc_addr);
    }
    */
    // 先检查要申请的内存在页目录表中是否存在
    temp = alloc_addr >> 22;
    temp <<= 2;
    temp |= 0xFFFFE000;

    // PrintHex(temp);

    if (((*(unsigned int *)temp) & 1) == 0)
    {
        /*
        {
            char str[] = "Not found in page index table\r\n";
            Puts(str);
        }
        */
        // 不在内存中
        unsigned int page_table = AllocatePhyPage(); // 申请物理页
        if (page_table == 0xFFFFFFFF)
        {
            SetEFLAGS(eflags);
            return -1; // 申请物理页失败
        }
        if (alloc_addr >= 0x80000000)
            *(unsigned int *)temp = page_table | 0x03;//内核页目录权限限制
        else
            *(unsigned int *)temp = page_table | 0x07; // 在页目录中注册该页表
        temp = alloc_addr & 0xFFC00000;
        temp >>= 10;
        temp |= 0xFF800000;
        // 将页表清空
        for (int i = 0; i < 1024; ++i)
            *((unsigned int *)temp + i) = 0;
        // 在最后一项填写自身地址
        //*((unsigned int *)temp + 1023) = (page_table & 0xFFFFF000) | 0x03;
    }
    // 检查页是否在页表中
    temp = 0xFF800000;
    temp |= (alloc_addr >> 12) << 2;
    // PrintHex(temp);
    if (((*(unsigned int *)temp) & 1) == 0)
    {
        /*
        {
            char str[] = "Not found in page table\r\n";
            Puts(str);
        }
        */
        // 页表中没有该物理页
        unsigned int page = AllocatePhyPage();
        if (page == 0xFFFFFFFF)
        {
            SetEFLAGS(eflags);
            return -1; // 申请物理页失败
        }
        *(unsigned int *)temp = page | attribute; // 普通页无需初始化
    }
    //将当前页目录倒数第二项清空
    temp = 0xFFFFF000 + 1022 * 4; // 指向当前页目录到数第二项
    *(unsigned int *)temp = 0;
    asm volatile(
        "invlpg (0xFFFFFFF8)\n\t"
        :
        :);
    /*
    {
        char str[] = "allocate finish\r\n";
        Puts(str);
    }
    */
    SetEFLAGS(eflags);
    return 0; // 分配成功
}
/*本函数未考虑中断，请事先关闭中断*/
void ResetTargetIndexTableItem(unsigned int cr3,unsigned int which)
{
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    //先将目标cr3载入当前cr3的倒数第二项
    unsigned int temp = 0xFFFFF000 + 1022 * 4; // 指向当前页目录到数第二项
    *(unsigned int *)temp = cr3 | 0x03;
    asm volatile(
        "invlpg (0xFFFFFFF8)\n\t"
        :
        :); // 刷新页目录表里该条目
    temp = 0xFFFFE000+(which<<2);
    *(int*)temp = 0;
     //将当前页目录倒数第二项清空
    temp = 0xFFFFF000 + 1022 * 4; // 指向当前页目录到数第二项
    *(unsigned int *)temp = 0;
    asm volatile(
        "invlpg (0xFFFFFFF8)\n\t"
        :
        :);
}

void FreePage(unsigned int page)
{
    page = page >> 12 << 12;
    unsigned int temp = page >> 22;
    temp <<= 2;
    temp |= 0xFFFFF000;
    if ((*(unsigned int *)temp) & 1)
    {
        // 页目录中存在此页表
        temp = (0xFFC00000|(page>>12<<2));
        if ((*(unsigned int *)temp) & 1)
        {
            // 物理页存在
            unsigned int *ptr = ((unsigned int *)temp);
            temp = *(unsigned int *)temp;
            FreePhyPage(temp); // 释放该页
            *ptr = 0;

            asm volatile(
            "invlpg (%%ebx)\n\t"
            :
            :"b"(page)
            );
        }
    }
}
void *kmalloc(unsigned int size)
{
	size += sizeof(unsigned int);
    int eflags = GetEFLAGS();
    CloseInterrupt();
    size = PALIGN_UP(size,4096);
    unsigned int page_num = size/4096;
    //开始寻找空闲页
    for (int i=0;i<KMALLOC_INFO.map_size_byte;++i)
    {
        if (KMALLOC_INFO.map[i]!=0xFF)
        {
            //这一页中存在空闲空间
            //检查连续空闲位是否充足
			for (int j = 0; j < 8; ++j)
			{
				if ((KMALLOC_INFO.map[i] & (1 << j)) == 0)
				{
					//这一位是空闲的
					int k = i * 8 + j;
					int count = 0;
					for (int l = k; l < KMALLOC_INFO.map_size_byte * 8; ++l)
					{
						if ((KMALLOC_INFO.map[l / 8] & (1 << (l % 8))) == 0)
						{
							count++;
							if (count == page_num)
							{
								//找到了连续的空闲页
                                char* ptr = (char*)(KMALLOC_INFO.start_vaddr + k * 4096);
                                char* ret = ptr;
								for (int m = k; m < k + count; ++m)
								{
									KMALLOC_INFO.map[m / 8] |= (1 << (m % 8));
									AllocatePage((unsigned int)ptr, 0x03);
									ptr += 4096;
								}
								SetEFLAGS(eflags);
								*(unsigned int*)ret = size;
								return (void*)(ret+sizeof(unsigned int));
							}
						}
						else
						{
							count = 0;
						}
					}
				}
			}
        }
    }
    SetEFLAGS(eflags);
}

void* kheapup(unsigned int size)
{
    int eflags = GetEFLAGS();
    CloseInterrupt();
    NEXT_ALLOCATE;
    unsigned int ret = NEXT_ALLOCATE;
    unsigned int page_num = PALIGN_UP(size, 0x1000) / 0x1000;
    while (page_num--)
    {
        AllocatePage(NEXT_ALLOCATE, 0x03);
        NEXT_ALLOCATE += 0x1000;
    }
    SetEFLAGS(eflags);
    return (void *)ret;
}

void kheapdown(unsigned int size)
{
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    unsigned int page_num = PALIGN_UP(size, 0x1000) / 0x1000;
    while (page_num--)
    {
        NEXT_ALLOCATE -= 0x1000;
        FreePage(NEXT_ALLOCATE);
    }
    SetEFLAGS(eflags);
}

void *heapup(unsigned int size)
{
    struct TASK_INFO_BLOCK *tb = GetNowTaskInfoPtr();
    unsigned int ret = tb->next_allocate;
    unsigned int page_num = PALIGN_UP(size, 0x1000) / 0x1000;

    while (page_num--)
    {
        AllocatePage(tb->next_allocate, 0x07);
        tb->next_allocate += 0x1000;
    }
    return (void *)ret;
}

void heapdown(unsigned int size)
{
    struct TASK_INFO_BLOCK *tb = GetNowTaskInfoPtr();
    unsigned int ret = tb->next_allocate;
    unsigned int page_num = PALIGN_UP(size, 0x1000) / 0x1000;
    while (page_num--)
    {
        tb->next_allocate -= 0x1000;
        FreePage(tb->next_allocate);
    }
}

void kfree(void *ptr)
{
    unsigned int row;
	unsigned int eflags = GetEFLAGS();
	CloseInterrupt();
	unsigned int page = (unsigned int)ptr;
	page -= sizeof(unsigned int);
    row = page;
    if (page % 4096 != 0) CloseCPU();
	unsigned int free_size = *(unsigned int*)page;
    page -= KMALLOC_INFO.start_vaddr;
	page /= 4096;
    free_size /= 4096;
	//从位图中释放
	for (int i = 0; i < free_size; ++i)
	{
		KMALLOC_INFO.map[page + i] &= ~(1 << (page % 8));
		FreePage(row);
        row += 4096;
        //__asm__ volatile ("xchg %bx,%bx");
	}
	SetEFLAGS(eflags);
}

unsigned int CopyPageIndexTable(int is_clear_user_space)
{
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    unsigned int temp = 0xFFFFF000 + 1022 * 4; // 指向当前页目录到数第二项
    int phypage_addr = AllocatePhyPage();      // 作为新的页目录表
    *(unsigned int *)temp = phypage_addr | 0x03;
    asm volatile(
        "invlpg (0xFFFFFFF8)\n\t"
        :
        :); // 刷新页目录表里该条目
    // 拷贝页目录表
    unsigned int new_page_index_table_addr = 0xFFFFE000;
    temp = 0xFFFFF000;
    for (int i = 0; i < 1022; ++i)
    {
        if (is_clear_user_space&&i<512)
            *(unsigned int *)new_page_index_table_addr=0;
        else
            *(unsigned int *)new_page_index_table_addr = *(unsigned int *)temp;
        temp += 4, new_page_index_table_addr += 4;
    }
    /*将新页目录表的最后一项更新为自身的地址，倒数第二项清空*/
    *(unsigned int *)new_page_index_table_addr = 0;
    new_page_index_table_addr += 4;
    *(unsigned int *)new_page_index_table_addr = phypage_addr | 0x03;
    /*将当前页目录表倒数第二项重新清零*/
    *(unsigned int *)temp = 0;
    /*重置当前页目录的用户空间*/
    //ResetUserNowPageIndexTable();
    SetEFLAGS(eflags);
    return phypage_addr;
}

void AsyncKernelPageIndexTable(unsigned int target_index_table_phy_addr)
{
    /*将当前进程的内核页目录区域同步到指定页目录的内核区域,不同步最后两项*/
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    unsigned int temp = 0xFFFFF000 + 1022 * 4;      // 指向当前页目录到数第二项
    int phypage_addr = target_index_table_phy_addr; // 作为新的页目录表
    *(unsigned int *)temp = phypage_addr | 0x03;
    asm volatile(
        "invlpg (0xFFFFFFF8)\n\t"
        :
        :);
    // 刷新页目录表里该条目
    // 拷贝页目录表
    unsigned int new_page_index_table_addr = 0xFFFFE000 + 512 * 4;
    temp = 0xFFFFF000 + 512 * 4;
    for (int i = 0; i < 510; ++i)
    {
        *(unsigned int *)new_page_index_table_addr = *(unsigned int *)temp;
        temp += 4, new_page_index_table_addr += 4;
    }
    /*将新页目录表的最后一项更新为自身的地址，倒数第二项清空*/
    *(unsigned int *)new_page_index_table_addr = 0;
    /*将当前页目录表倒数第二项重新清零*/
    *(unsigned int *)temp = 0;
    SetEFLAGS(eflags);
}

int mmap(unsigned int v_addr, unsigned int phy_addr, unsigned int attribute)
{
    // 清空物理页低12位
    phy_addr &= 0xFFFFF000;
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    // 先标记物理页已被使用，防止后续申请页表时使用了该物理页
    SignPhyPageUsed(phy_addr);
    // 先检查要申请的内存在页目录表中是否存在
    unsigned int temp = v_addr >> 22;
    temp <<= 2;
    temp |= 0xFFFFF000;

    // PrintHex(temp);

    if (((*(unsigned int *)temp) & 1) == 0)
    {
        // 不在页目录中
        unsigned int page_table = AllocatePhyPage(); // 申请物理页作为页表
        if (page_table == 0xFFFFFFFF)
        {
            SetEFLAGS(eflags);
            return -1; // 申请物理页失败
        }
        *(unsigned int *)temp = page_table | 0x07; // 在页目录中注册该页
        temp = v_addr & 0xFFC00000;
        temp >>= 10;
        temp |= 0xFFC00000;
        // 将页表清空
        for (int i = 0; i < 1024; ++i)
            *((unsigned int *)temp + i) = 0;
    }
    // 检查页是否在页表中
    temp = 0xFFC00000;
    temp |= (v_addr >> 12) << 2;
    PrintHex(temp);
    if (((*(unsigned int *)temp) & 1) == 1)
    {
        FreePhyPage(*(unsigned int *)temp);
    }

    // 页表中没有该物理页
    unsigned int page = phy_addr;
    *(unsigned int *)temp = page | attribute; // 普通页无需初始化

    FlushCR3();
    SetEFLAGS(eflags);
    return 0; // 分配成功
}

void SignPhyPageUsed(unsigned int phy_addr)
{
    phy_addr >>= 12;
    int i = phy_addr / 32, j = phy_addr % 32;
    unsigned int *t = (unsigned int *)PHY_PAGE_TABLE_ADDR;
    t[i] |= (1 << (32 - 1 - j));
}

unsigned int *GetKernelPageIndexTable(void)
{
    return KERNEL_PAGE_INDEX_TABLE_ADDR;
}

void SetKernelPageIndexTable(unsigned int *index_table_v_addr)
{
    KERNEL_PAGE_INDEX_TABLE_ADDR = index_table_v_addr;
}

void ResetUserNowPageIndexTable(void)
{
    /*
    for (int i = 0; i < 512; ++i)
    {
        GetKernelPageIndexTable()[i] = 0;
    }
    */
    unsigned int temp = 0xFFFFF000;
    for (int i = 0; i < 512; ++i)
    {
        *(unsigned int *)temp=0;
        temp += 4;
    }
    FlushCR3();
}

void ResetKernelUserSpace(void)
{
    for (int i = 0; i < 512; ++i)
    {
        GetKernelPageIndexTable()[i] = 0;
    }
    FlushCR3();
}

void AsyncNowKernelSpaceIndexTableToKernelSpace(void)
{
    unsigned int *p = (unsigned int *)0xFFFFF000;
    DisableSwitchTask();
    unsigned int eflag = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    unsigned int *k = GetKernelPageIndexTable();
    for (int i = 512; i < 1023; ++i)
    {
        k[i] = p[i];
    }
    SetEFLAGS(eflag);
}

void AsyncKernelSpaceIndexTableToNowKernelSpace(void)
{
    unsigned int *p = (unsigned int *)0xFFFFF000;
    DisableSwitchTask();
    unsigned int eflag = GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    unsigned int *k = GetKernelPageIndexTable();
    for (int i = 512; i < 1023; ++i)
    {
        p[i] = k[i];
    }
    SetEFLAGS(eflag);
}
//初始化内核malloc,调用本函数后请勿手动使用kheap相关函数
void kmalloc_init(unsigned int max_size)
{
    if (max_size>(unsigned int)1024*1024*1024) CloseCPU();
    //计算需要的位图大小
    unsigned int map_size = (max_size/4096)/8;
    KMALLOC_INFO.map_size_byte = map_size;
    KMALLOC_INFO.map = kheapup(map_size);
    KMALLOC_INFO.start_vaddr = (unsigned int)kheapup(1);
    kheapdown(1);
    for (int i=0;i<map_size;++i) KMALLOC_INFO.map[i]=0;
}
