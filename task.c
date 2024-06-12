
#include "task.h"
#include "c_basic.h"
#include "screen_ctrl.h"
#include "memory.h"
#include "task_basic_info.h"
#include "base_io.h"
static struct TASK_INFO_BLOCK *NOW_TASK_ADDR;
static unsigned int time_counter = 0;
static int IS_SWITCH_TASK_ENABLE = 1;
static int TASK_SWITCH_FLAGS[128]={1};

struct DESCRIPTOR MakeSegmentDescriptor(void *base_addr, unsigned int segment_limit, unsigned short int type)
{
    struct DESCRIPTOR des;
    unsigned int temp;
    des.low = (unsigned int)base_addr;
    des.low <<= 16;
    des.low |= (segment_limit & 0xFFFF);
    des.height = (unsigned int)base_addr;
    des.height &= 0xFF000000;
    type &= 0xF0FF;
    des.height |= (type << 8);
    des.height |= (segment_limit & 0xF0000);
    des.height |= (((unsigned int)base_addr >> 16) & 0xFF);
    return des;
}
struct DESCRIPTOR MakeGateDescriptor(unsigned int offset, unsigned short int seg, int p, int dpl, int type, int par_num)
{
    struct DESCRIPTOR des = {0};
    des.height = offset;
    des.height &= 0xFFFF0000;
    des.height |= p << 15;
    des.height |= dpl << 13;
    des.height |= type << 8;
    des.height |= par_num;
    seg <<= 3;
    des.low = seg;
    des.low <<= 16;
    des.low |= (offset & 0x0000FFFF);
    return des;
}

unsigned short int InstallGDT(struct DESCRIPTOR des, struct DESCRIPTOR_REG *addr)
{
    if (addr->segment_limit <= 3)
        return -1; // 初始段界限错误
    struct DESCRIPTOR *ptr = (struct DESCRIPTOR *)(addr->base_addr + addr->segment_limit + 1);
    *ptr = des;
    ptr += 1;
    addr->segment_limit = (unsigned int)ptr - (unsigned int)addr->base_addr - 1;
    int n = ptr - (struct DESCRIPTOR *)addr->base_addr;
    n -= 1;
    unsigned short int res;
    res = n << 3;
    return res;
}

void FlashGDT(struct DESCRIPTOR_REG *addr)
{
    asm volatile(
        "lgdt (%%edx)\n\t"
        :
        : "d"(addr));
}

void InitKernelTask()
{
    // 初始化TASK_BLOCK
    TASK_SWITCH_FLAGS[0] = 1;
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    struct TASK_INFO_BLOCK *ptr = (struct TASK_INFO_BLOCK *)KERNEL_TASK_BLOCK_ADDR;
    ptr->cr3 = 0x80020000;
    ptr->cs = 0x08;
    ptr->ds = 0x10;
    ptr->es = 0x10;
    ptr->ss = 0x10;
    ptr->fs = 0x10;
    ptr->gs = 0x10;
    ptr->next = ptr;
    ptr->next_allocate = 0x80201000;
    ptr->task_id = 0;
    //ptr->extra_info=&(ptr->_extra);
    ptr->task_state = 1;
    {
        char str[] = "kernel";
        Strcpy(ptr->task_name, str);
    }

    // 设置TSS
    struct TSS *tss = (struct TSS *)TSS_ADDR;
    for (int i = 0; i < 32; ++i)
        *((int *)TSS_ADDR + i) = 0;
    tss->CR3 = ptr->cr3;
    tss->CS = ptr->cs;
    tss->DS = ptr->ds;
    tss->ES = ptr->es;
    tss->GS = ptr->gs;
    tss->FS = ptr->fs;
    tss->SS = ptr->ss;
    tss->SS0 = 0x10;
    tss->IO_BASE = 103;
    // 在GDT安装TSS描述符
    struct DESCRIPTOR_REG reg;
    reg.base_addr = 0x80008000;
    reg.segment_limit = 39;
    unsigned short int seg = InstallGDT(MakeSegmentDescriptor((void *)TSS_ADDR, 103, 0x89), &reg);
    FlashGDT(&reg);
    // WaitForInterrupt();
    __asm__ volatile(
        "ltr %%ax\n\t"
        :
        : "a"(seg));
    SetNowTaskInfoPtr((struct TASK_INFO_BLOCK *)KERNEL_TASK_BLOCK_ADDR);
    SetEFLAGS(eflags);
    RecoverSwitchTask();
}

void switch_task(void)
{
    CloseInterrupt();//本函数调用一定从中断中引发，从中断返回时可还原eflags，无需备份状态
    time_counter+=1;
    if (!IS_SWITCH_TASK_ENABLE) return;
    struct TASK_INFO_BLOCK *tb = GetNowTaskInfoPtr();
    /*寻找下一个可用的空闲任务*/
    struct TASK_INFO_BLOCK *temp = tb->next;
    while (temp != tb)
    {
        //__asm__ volatile("xchg %%bx,%%bx\n\t":);
        if (temp->task_state == 3 && temp->wait_to_time!=0xFFFFFFFF)
        {
            temp->wait_to_time-=1;
            //Puts("Sub wait time\r\n");
            if (temp->wait_to_time==0) temp->task_state = 0;
        }
        if (temp->task_state == 0)
        {

            /*切换到这个任务*/
            /*需要将当前任务的ss esp ebp cs eip eflags cr3保存，其他部分均无需保存*/
            /*将下次恢复时的地址保存在当前任务的任务块中*/
            void *recover_addr;
            __asm__ volatile(
                "mov $RECOVER_TASK,%%eax\n\t"
                //"add $0x80040000,%%eax\n\t"
                : "=a"(recover_addr)
                :);
            tb->eip = (unsigned int)recover_addr;
            /*获取esp寄存器的内容并保存*/
            unsigned int regist;
            __asm__ volatile(
                "mov %%esp,%%eax\n\t"
                : "=a"(regist));
            tb->esp = regist;
            /*获取ebp寄存器*/
            __asm__ volatile(
                "mov %%ebp,%%eax\n\t"
                : "=a"(regist));
            tb->ebp = regist;
            __asm__ volatile(
                "mov %%cr3,%%eax\n\t"
                : "=a"(regist));
            tb->cr3 = regist;
            /*获取ss段寄存器*/
            unsigned short int ss_register, cs_reegister;
            __asm__ volatile(
                "xor %%eax,%%eax\n\t"
                "mov %%ss,%%ax\n\t"
                : "=a"(ss_register)
                :);
            tb->ss = ss_register;
            /*获取cs段寄存器*/
            __asm__ volatile(
                "xor %%eax,%%eax\n\t"
                "mov %%cs,%%ax\n\t"
                : "=a"(cs_reegister));
            tb->cs = cs_reegister;
            __asm__ volatile(
                "xor %%eax,%%eax\n\t"
                "mov %%ds,%%ax\n\t"
                : "=a"(tb->ds));
            __asm__ volatile(
                "xor %%eax,%%eax\n\t"
                "mov %%es,%%ax\n\t"
                : "=a"(tb->es));
            __asm__ volatile(
                "xor %%eax,%%eax\n\t"
                "mov %%fs,%%ax\n\t"
                : "=a"(tb->fs));
            __asm__ volatile(
                "xor %%eax,%%eax\n\t"
                "mov %%gs,%%ax\n\t"
                : "=a"(tb->gs));
            /*备份eflags寄存器*/
            tb->EFLAGS = GetEFLAGS();
            /*切换当前任务指针，并设置任务状态*/
            temp->task_state = 1;
            if (tb->task_state == 1)
                tb->task_state = 0;
            SetNowTaskInfoPtr(temp);
            /*更改tss的esp0*/
            /*将目标任务的esp0放在tss中*/
            ((struct TSS *)TSS_ADDR)->ESP0 = temp->esp0;
            /*利用iret跳转到目标任务,依据跳转到目标段是否要切换栈构造方式不同*/
            if (temp->ss & 3 == 3)
            {
                /*需要切换到3特权级栈,此情况可能是任务初次执行*/
                __asm__ volatile(
                    "mov 50(%%ebx),%%ax\n\t"
                    "mov %%ax,%%ds\n\t"
                    "mov 48(%%ebx),%%ax\n\t"
                    "mov %%ax,%%es\n\t"
                    "mov 46(%%ebx),%%ax\n\t"
                    "mov %%ax,%%fs\n\t"
                    "mov 44(%%ebx),%%ax\n\t"
                    "mov %%ax,%%gs\n\t"
                    "mov 28(%%ebx),%%ebp\n\t"
                    "mov 36(%%ebx),%%eax\n\t"
                    "mov %%eax,%%cr3\n\t"
                    "mov 56(%%ebx),%%esp\n\t"
                    "pushl 52(%%ebx)\n\t"
                    "pushl 24(%%ebx)\n\t"
                    "pushl 32(%%ebx)\n\t"
                    "pushl 54(%%ebx)\n\t"
                    "pushl 40(%%ebx)\n\t"
                    "iret\n\t"
                    :
                    : "b"(temp));
            }
            else
            {
                /*无需切换栈*/
                __asm__ volatile(
                    "mov 50(%%ebx),%%ax\n\t"
                    "mov %%ax,%%ds\n\t"
                    "mov 48(%%ebx),%%ax\n\t"
                    "mov %%ax,%%es\n\t"
                    "mov 46(%%ebx),%%ax\n\t"
                    "mov %%ax,%%fs\n\t"
                    "mov 44(%%ebx),%%ax\n\t"
                    "mov %%ax,%%gs\n\t"
                    "mov 28(%%ebx),%%ebp\n\t"
                    "mov 36(%%ebx),%%eax\n\t"
                    "mov %%eax,%%cr3\n\t"
                    "mov 24(%%ebx),%%esp\n\t"
                    "mov 52(%%ebx),%%ax\n\t"
                    "mov %%ax,%%ss\n\t"
                    "pushl 32(%%ebx)\n\t"
                    "pushl 54(%%ebx)\n\t"
                    "pushl 40(%%ebx)\n\t"
                    "iret\n\t"
                    :
                    : "b"(temp));
            }
        }
        else if (temp->task_state == 4)
        {
            // 清除该进程的数据
            struct TASK_INFO_BLOCK *p = temp->next;
            CleanTask(temp->task_id);
            temp = p;
            continue;
        }

        temp = temp->next;
    }

    /*恢复当前任务时从这里开始执行*/
    /*定义标签*/
    asm volatile("RECOVER_TASK:");
    return;
}
int ReadProgramSize(unsigned int sector)
{
    char temp[512];
    ReadSector(temp, sector, 1);
    return *(int *)temp;
}
int ReadProgram(char *buffer, unsigned int buffer_size, unsigned int sector)
{
    char temp[512];
    ReadSector(temp, sector, 1);
    int program_len = *(int *)temp - 4;
    if (buffer_size < program_len)
        return -1;
    if (program_len <= 512 - 4)
    {
        Memcpy(buffer, temp + 4, program_len);
        return 0;
    }
    else
    {
        Memcpy(buffer, temp + 4, 512 - 4);
        program_len -= 512 - 4;
        buffer += 512 - 4;
    }
    sector += 1;
    while (program_len > 0)
    {
        if (program_len > 512)
        {
            ReadSector(temp, sector++, 1);
            Memcpy(buffer, temp, 512);
            buffer += 512;
            program_len -= 512;
        }
        else
        {
            ReadSector(temp, sector, 1);
            Memcpy(buffer, temp, program_len);
            return 0;
        }
    }
    return 0;
}
//创建进程，指定进程所在扇区、权限等级及进程附加到的session
int LoadTask(int sector, int dpl,struct Session* session)
{
    unsigned int eflags;
    // 获取目标程序大小
    int program_size = ReadProgramSize(sector);
    Session_PutString(session,"Program size:");
    Session_PrintNumber(session,program_size,10);
    if (program_size <= 0)
        return -1;
    // 为该程序创建基本信息
    struct TASK_INFO_BLOCK *tb = (struct TASK_INFO_BLOCK *)kmalloc(sizeof(struct TASK_INFO_BLOCK) + 30);
    Memset(tb, 0, sizeof(struct TASK_INFO_BLOCK) + 30);
    /*为进程块申请额外信息*/
    tb->extra_info = &tb->_extra;
    ((struct TASK_EXTRA_INFO*)tb->extra_info)->session = session;
    //((struct TASK_EXTRA_INFO*)tb->extra_info)->
    /*将页目录表低512位清零*/
    /*
    unsigned int *now_page_index_table_ptr = (unsigned int *)0xFFFFF000;
    for (int i = 0; i < 512; ++i)
        now_page_index_table_ptr[i] = 0;
    */
    //备份当前进程的CR3
    int now_cr3;
    __asm__ volatile (
        "movl %%cr3,%%eax\n\t"
        :"=a"(now_cr3)
        :
    );
    // 为新进程创建页表
    int cr3 = CopyPageIndexTable(0);
    eflags = GetEFLAGS();
    CloseInterrupt();//在此期间不允许任务切换
    //将当前页目录切换到新的进程
    __asm__ volatile (
        "movl %%eax,%%cr3\n\t"
        :
        :"a"(cr3)
    );
    //将页目录用户空间除0级栈以外的内容清空
    unsigned int temp = 0xFFFFF000;
    for (int i = 0; i < 511; ++i)
    {
        *(unsigned int *)temp=0;
        temp += 4;
    }
    FlushCR3();
    // 为程序申请除0级栈以外内存,程序代码段和用户栈在低地址空间，0级栈在用户空间顶端
    int i, k;
    for (i = 0; i < (program_size - 1) / 4096 + 1; ++i)
    {
        AllocatePage((i+1) * 4096, 0x05); // 只读部分
    }
    ReadProgram((char *)0x1000, i * 4096, sector); // 读取程序本身
    // 设置esp0，该值为定值，分配内存将在CR3切换回原本进程后进行
    tb->esp0 = 0x80000000;
    for (k = 0; k < 256; ++k)
    {
        AllocatePage((1+i++) * 4096, 0x07); // 分配3级栈
    }
    tb->esp = (i+1) * 4096;
    tb->next_allocate = tb->esp;
    tb->task_id = AllocateTaskID(); /*为用户程序申请ID*/
    /*拷贝页目录给用户*/
    tb->cr3 = cr3;
    //切换回原本的CR3
    __asm__ volatile (
        "movl %%eax,%%cr3\n\t"
        :
        :"a"(now_cr3)
    );
    //将目标进程的0级栈清空
    ResetTargetIndexTableItem(cr3,511);
    //为目标进程分配0级栈
    unsigned int end = 0x7ff00000;//0级栈最后一个页地址，是第524,287个页（0开始计数）
    for (k = 0; k < 256; ++k)
    {
        AllocatePageInOtherProcress(end, 0x03,cr3); // 分配0级栈
        end+=4096;
    }
    //恢复中断状态
    SetEFLAGS(eflags);
    /*设置用户初始状态信息*/
    tb->cs = 0x1B;
    tb->ds = 0x23;
    tb->es = 0x23;
    tb->fs = 0x23;
    tb->gs = 0x23;
    tb->ss = 0x23;
    Strcpy(tb->task_name, "User program");
    tb->eip = 0x1000; /*用户程序起始执行位置*/
    tb->task_state = 0;
    /*将任务加入任务链，先关闭中断*/
    DisableSwitchTask();
    eflags = GetEFLAGS();
    tb->EFLAGS = eflags; /*新任务初始eflags状态继承自内核任务*/
    CloseInterrupt();
    struct TASK_INFO_BLOCK *head = GetNowTaskInfoPtr();
    tb->next = head->next;
    head->next = tb;
    Session_AttachTask(session,tb->task_id);
    SetEFLAGS(eflags);
    Session_PutString(session,"New process created\r\n");
    RecoverSwitchTask();
}

unsigned int AllocateTaskID(void)
{
    DisableSwitchTask();
    unsigned int eflage = GetEFLAGS();
    CloseInterrupt();
    unsigned int id = 1; // 从ID1开始,0必然被内核占用
    int sign;
    for (id; 1; ++id)
    {
        if (id > 100000)
        {
            Puts("[SYSTEM ERROR] Too many task created!\r\n");
            WaitForInterrupt();
        }
        sign = 0;
        struct TASK_INFO_BLOCK *temp = GetNowTaskInfoPtr();
        do
        {
            if (temp->task_id == id)
            {
                sign = 1;
                break;
            }
            temp = temp->next;
        } while (temp != GetNowTaskInfoPtr());
        if (sign == 0)
            break;
    }
    RecoverSwitchTask();
    SetEFLAGS(eflage);
    return id;
}

void CleanTask(unsigned int task_id)
{
    if (task_id == 0)
        return; /*不允许清除内核任务*/
    /*不允许清除当前任务本身*/
    if (GetNowTaskInfoPtr()->task_id == task_id)
        return;
    struct TASK_INFO_BLOCK *wait_clean = 0;
    int sign = 0;
    /*操作链表过程不能打断*/
    DisableSwitchTask();
    unsigned int eflage = GetEFLAGS();
    CloseInterrupt();
    struct TASK_INFO_BLOCK *temp = GetNowTaskInfoPtr();
    do
    {
        if (temp->next->task_id == task_id)
        {
            wait_clean = temp->next;
            sign = 1;
            /*将该任务从任务链表中取出*/
            temp->next = temp->next->next;
            /*无需考虑改变当前任务指针，因为被清理的一定不是当前任务*/
            /*无需考虑只有一个任务的情况，因为内核任务必然存在且不可被清理*/
            break;
        }
        temp=temp->next;
    } while (temp != GetNowTaskInfoPtr());
    RecoverSwitchTask();
    SetEFLAGS(eflage);
    /*后续是正常清理过程，允许被中断*/
    if (!sign)
        return; /*没有找到这个任务*/
    /*将待清理任务的页目录放在当且页目录倒数第二项*/
    unsigned int now_index_page = 0xFFFFF000 + 1022 * 4; // 指向当前页目录到数第二项
    *(unsigned int *)now_index_page = (wait_clean->cr3) | 0x03;
    asm volatile(
        "invlpg (0xFFFFFFF8)\n\t"
        :
        :);
    unsigned int task_page_index_table_addr = 0xFFFFF000;
    unsigned int page_table_addr;
    for (int i = 0; i < 512; ++i)
    {
        if (*((unsigned int *)task_page_index_table_addr + i) & 1 == 1)
        {
            /*该页表存在*/
            page_table_addr = 0xFFC00000 | (i << 12);
            // asm volatile ("xchg %bx,%bx");
            // 释放页表中的每一项
            for (int k = 0; k < 1024; ++k)
            {
                if (*((unsigned int *)page_table_addr + k) & 1 == 1)
                {
                    /*该页存在*/
                    FreePhyPage(*((unsigned int *)page_table_addr + k));
                }
            }
            /*释放页表*/
            FreePhyPage(*((unsigned int *)task_page_index_table_addr + i));
        }
    }
    /*删除当前任务页目录倒数第二项*/
    *(unsigned int *)now_index_page = 0;
    /*释放页目录表*/
    FreePhyPage(wait_clean->cr3);
    /*释放进程块*/
    kfree(wait_clean);
}

void CloseNowTask(void)
{
    /*关闭中断以访问当前软件链表，将当前软件设置为4等待回收*/
    DisableSwitchTask();
    unsigned int eflage = GetEFLAGS();
    CloseInterrupt();
    struct TASK_INFO_BLOCK* info = GetNowTaskInfoPtr();
    info->task_state = 4;
    /*将当前进程分离*/
    if (((struct TASK_EXTRA_INFO*)info->extra_info)->session)
        Session_DetachTask(((struct TASK_EXTRA_INFO*)info->extra_info)->session,info->task_id);
    /*恢复中断并进入死循环等待被回收*/
    RecoverSwitchTask();
    SetEFLAGS(eflage);
    SwitchToNextTask();
}

int PauseTask(unsigned int task_id)
{
    DisableSwitchTask();
    unsigned int eflags = GetEFLAGS();
    CloseInterrupt();
    struct TASK_INFO_BLOCK *p;
    if (0xFFFFFFFF == (unsigned int)(p = GetTaskInfo(task_id)))
    {
        RecoverSwitchTask();
        SetEFLAGS(eflags);
        return -1;
    }
    p->task_state = 3; // 暂停
    RecoverSwitchTask();
    SetEFLAGS(eflags);
    return 0;
}
//若未找到目标进程则返回0xFFFFFFFF(-1)
struct TASK_INFO_BLOCK *GetTaskInfo(unsigned int task_id)
{

    struct TASK_INFO_BLOCK *p = GetNowTaskInfoPtr(), *t;
    if (p->task_id == task_id)
        return p;
    for (t = p->next; p != t; t = t->next)
    {
        if (t->task_id == task_id)
            return t;
    }
    return (struct TASK_INFO_BLOCK *)0xFFFFFFFF;
}
//返回当前正在执行的进程，本函数不确保进程安全，需要关闭中断或保证进程列表不会修改
struct TASK_INFO_BLOCK *GetNowTaskInfoPtr(void)
{
    return NOW_TASK_ADDR;
}

void SetNowTaskInfoPtr(struct TASK_INFO_BLOCK *ptr)
{
    NOW_TASK_ADDR = ptr;
}

void DisableSwitchTask()
{
    
    if (TASK_SWITCH_FLAGS[0]>=128)
    {
        CloseInterrupt();
        WaitForInterrupt();
    }
    TASK_SWITCH_FLAGS[TASK_SWITCH_FLAGS[0]++] = IS_SWITCH_TASK_ENABLE;
    IS_SWITCH_TASK_ENABLE = 0;
    
}

void RecoverSwitchTask()
{
    IS_SWITCH_TASK_ENABLE = TASK_SWITCH_FLAGS[--TASK_SWITCH_FLAGS[0]];
}
//休眠当前进程，单位毫秒，若为0xFFFFFFFF则无限制休眠
void SleepNowTask(unsigned int time_ms)
{
    unsigned int eflags=GetEFLAGS();
    CloseInterrupt();
    GetNowTaskInfoPtr()->wait_to_time = time_ms/2;
    GetNowTaskInfoPtr()->task_state = 3;
    SetEFLAGS(eflags);
    SwitchToNextTask();
}
//唤醒进程，将会重置定时器并将进程状态改为运行状态
void WakeUpTask(unsigned int task_id)
{
    DisableSwitchTask();
    unsigned int eflags=GetEFLAGS();
    CloseInterrupt();
    if (GetNowTaskInfoPtr()->task_id == task_id)
    {
        RecoverSwitchTask();
        SetEFLAGS(eflags);
        return;
    }
    struct TASK_INFO_BLOCK*p = GetTaskInfo(task_id);
    if (p!=(void*)0xFFFFFFFF)
    {
        p->task_state = 0;
        p->wait_to_time = 0;
    }
    RecoverSwitchTask();
    SetEFLAGS(eflags);
}

//本函数将设置进程状态为无限等待，直到键盘输入事件发生,返回键盘扫描码。
//若在中断关闭的情况下调用将会导致CPU停止工作
int WaitForKeyboardInput()
{
    //获取当前任务并将其指示为等待输入状态，并进入无限等待
    int eflags = GetEFLAGS();
    void CloseInterrupt();
    GetNowTaskInfoPtr()->is_wait_key = 1;
    SetEFLAGS(eflags);
    SleepNowTask(0xFFFFFFFF);
    //恢复执行时输入已被存储在进程的输入缓冲区中
    //关中断并获取输入
    eflags = GetEFLAGS();
    CloseInterrupt();
    int scancode = ((struct TASK_EXTRA_INFO*)(GetNowTaskInfoPtr()->extra_info))->last_input_keyscan_code;
    SetEFLAGS(eflags);
    return scancode;
}
//切换到下一个进程，利用软中断立即引发int 0x70
void SwitchToNextTask()
{
    __asm__ volatile ("int $0x70");
}


int GetTaskNumber()
{
    struct TASK_INFO_BLOCK* p = (struct TASK_INFO_BLOCK*)NOW_TASK_ADDR->next;
    int i=1;
    while (p!=NOW_TASK_ADDR) ++i;
    return i;
}

void* TaskHeapUp(unsigned int size)
{
    return heapup(size);
}

void TaskHeapDown(unsigned int size)
{
    return heapdown(size);
}