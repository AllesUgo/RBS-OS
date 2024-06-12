#include "base_io.h"
#include "screen_ctrl.h"
#include "Interrupt.h"
#include "task.h"
#include "memory.h"
#include "time.h"
#include "keybord.h"
#include "basic_info.h"
#include "session.h"
#include "c_basic.h"
#define cpuid(func,eax,ebx,ecx,edx)\
    asm volatile ("cpuid":\
    "=a" (eax),"=b" (ebx),"=c" (ecx),"=d" (edx):\
    "a" (func));






void rbsmain()
{
    CloseInterrupt();//必须关闭中断
    CleanScreen();
    Puts("RBS OS Kernel is loading...\r\n");
    struct DESCRIPTOR_REG reg;

    reg.base_addr = IDT_TABLE_ADDRESS;
    reg.segment_limit = 2047;
    // 内核起点
    InitPhyPageTable();//初始化页分配表
    InitKernelAllIndexTable();//为内核空间所有页目录分配页表
    InitKernelTask();//创建内核任务
    Puts("Kernel task create OKYA\r\n");
    Puts("Start init interrupt vector table\r\n");
    struct DESCRIPTOR des = MakeGateDescriptor((unsigned int)int_default , 1, 1, 0, 0xE, 0);
    for (int i = 20; i < 256; i++)
    {
        if (InstallInterrupt(i, des, &reg))
        {
            char str[] = "Install interrupt error\r\n";
            Puts(str);
            CloseInterrupt();
            WaitForInterrupt();
        }
    }
    des = MakeGateDescriptor((unsigned int)int0  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(0, des, &reg);
    des = MakeGateDescriptor((unsigned int)int1  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(1, des, &reg);
    des = MakeGateDescriptor((unsigned int)int2  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(2, des, &reg);
    des = MakeGateDescriptor((unsigned int)int3  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(3, des, &reg);
    des = MakeGateDescriptor((unsigned int)int4  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(4, des, &reg);
    des = MakeGateDescriptor((unsigned int)int5  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(5, des, &reg);
    des = MakeGateDescriptor((unsigned int)int6  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(6, des, &reg);
    des = MakeGateDescriptor((unsigned int)int7  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(7, des, &reg);
    des = MakeGateDescriptor((unsigned int)int8  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(8, des, &reg);
    des = MakeGateDescriptor((unsigned int)int9  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(9, des, &reg);
    des = MakeGateDescriptor((unsigned int)int10  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(10, des, &reg);
    des = MakeGateDescriptor((unsigned int)int11  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(11, des, &reg);
    des = MakeGateDescriptor((unsigned int)int12  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(12, des, &reg);
    des = MakeGateDescriptor((unsigned int)int13  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(13, des, &reg);
    des = MakeGateDescriptor((unsigned int)int14  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(14, des, &reg);
    des = MakeGateDescriptor((unsigned int)int15  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(15, des, &reg);
    des = MakeGateDescriptor((unsigned int)int16  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(16, des, &reg);
    des = MakeGateDescriptor((unsigned int)int17  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(17, des, &reg);
    des = MakeGateDescriptor((unsigned int)int18  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(18, des, &reg);
    des = MakeGateDescriptor((unsigned int)int19  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(19, des, &reg);
    des = MakeGateDescriptor((unsigned int)int20  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(20, des, &reg);
    
    des = MakeGateDescriptor((unsigned int)int0x70  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(0x70, des, &reg);//设置时钟中断
    des = MakeGateDescriptor((unsigned int)int0x66  , 1, 1, 3, 0xF, 0);
    InstallInterrupt(0x66, des, &reg);
    des = MakeGateDescriptor((unsigned int)int0x21  , 1, 1, 0, 0xE, 0);
    InstallInterrupt(0x21, des, &reg);

    FlashInterrupt(&reg);
    Puts("Flash interrupt vector table OKYA\r\n");
    kmalloc_init(1024*1024*1024);
    Puts("Init kernel malloc info\r\n");
    EnableTimeInterrupt();
    EnableKeybordInterrupt();
    Init8259A();
    StartInterrupt();
    Puts("Enable Interrupt\r\n");
    Puts("Mmap kernel page index table\r\n");
    mmap(KERNEL_PAGE_INDEX_TABLE_V_ADDR,KERNEL_PAGE_INDEX_TABLE_PHY_ADDR,0x03);
    SetKernelPageIndexTable((unsigned int*)KERNEL_PAGE_INDEX_TABLE_V_ADDR);
    //清空内核低2G的页表
    ResetKernelUserSpace();
    char str5[512];
    Puts("Now test disk reader...\r\n");
    ReadSector(str5, 20, 1);
    Puts("Disk read test finished\r\n");
    {
        char *temp = (char *)kmalloc(1024 * 1024);
        *temp = 'A';
        *(temp + 4096) = 'S';
        //kheapdown(temp);
        kfree(temp);
        char* p = (char*)kmalloc(1024*1023);
        if (p!=temp)
        {
            Puts("Memory test error!\r\n");
            CloseCPU();
        }
        kfree(p);
        char str[] = "TestMemoryOK\r\n";
        Puts(str);
    }
    //LoadTask(100, 3);
    {
        /*显示CPU信息*/
        unsigned int eflages=GetEFLAGS();
        CloseInterrupt();
        char*p;
        unsigned int arr[12];
        cpuid(0x80000002,arr[0],arr[1],arr[2],arr[3]);
        cpuid(0x80000003,arr[4],arr[5],arr[6],arr[7]);
        cpuid(0x80000004,arr[8],arr[9],arr[10],arr[11]);
        p=(char*)arr;
        for (int i=0;i<16*3;i++)
        {
            Putchar(p[i],0x07);
        }
        Putchar('\r',7);
        Putchar('\n',7);
        SetEFLAGS(eflages);
    }
    struct Session* session = Session_CreateSession();
    Puts("Kernel session created\r\n");
    Session_SwitchToSession(session);
    Puts("Session Switched\r\nPress keyboard to input\r\n");
    //LoadTask(66, 3,session);
    Session_PutString(session,"Kernel finished\r\n");
    //Session_PrintNumber(session,(int)Session_PrintNumber,16);
    LoadTask(100,3,session);
    while (1)
    {
        WaitForInterrupt();
    }
}