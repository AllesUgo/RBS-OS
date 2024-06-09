#include "Interrupt.h"
#include "base_io.h"
#include "task.h"
#include "keybord.h"
#include "c_basic.h"
#include "screen_ctrl.h"
#include "basic_info.h"
#include "base_type.h"
int InstallInterrupt(int interrupt_id, struct DESCRIPTOR des, struct DESCRIPTOR_REG *addr)
{
    // 中断向量表大小
    int INTERRUPT_TABLE_SIZE = 256;

    // 检查中断号是否有效
    if (interrupt_id >= INTERRUPT_TABLE_SIZE)
        return -1;
    struct DESCRIPTOR *ptr = (struct DESCRIPTOR *)(addr->base_addr + interrupt_id * 8);
    *ptr = des;
    return 0;
}
void FlashInterrupt(struct DESCRIPTOR_REG *addr)
{
    asm volatile(
        "lidt (%%edx)\n\t"
        :
        : "d"(addr));
}
void Init8259A()
{
    WritePort(0x20, 0x11);
    WritePort(0x21, 0x20);
    WritePort(0x21, 0x04);
    WritePort(0x21, 0x01);
    WritePort(0xA0, 0x11);
    WritePort(0xA1, 0x70);
    WritePort(0xA1, 0x02);
    WritePort(0xA1, 0x01);
}
void __cdecl deal_soft_interrupt(unsigned int num, unsigned int b, unsigned int c, unsigned int d)
{
    switch (num)
    {
    case 0:
    {
        struct TASK_INFO_BLOCK *info = GetNowTaskInfoPtr();
        if (((struct TASK_EXTRA_INFO *)info->extra_info)->session)
        {
            Session_PutString(((struct TASK_EXTRA_INFO *)info->extra_info)->session, (const char *)b);
        }
    }
    break;
    case 1:
    {
        struct TASK_INFO_BLOCK *info = GetNowTaskInfoPtr();
        if (((struct TASK_EXTRA_INFO *)info->extra_info)->session)
        {
            Session_PrintNumber(((struct TASK_EXTRA_INFO *)info->extra_info)->session, b,c);
        }
    }
    break;
    case 2:
        CloseNowTask();
        /*终止进程*/
    case 3:
        SleepNowTask(b);
        break;
    case 4:
        *((unsigned int *)b) = WaitForKeyboardInput();
        break;
    default:
        Puts("Unknown soft interrupt:");
        PrintHex(num);
        CloseInterrupt();
        WaitForInterrupt();

        break;
    }
}

void DealKeyboartInterrupt(void)
{
    unsigned char status, keycode;
    // 读取键盘控制器状态寄存器
    status = ReadPort(0x64);
    // 检查键盘缓冲区是否有数据可供读取
    if (status & 0x01)
    {
        // 读取键盘扫描码或字符数据
        keycode = ReadPort(0x60);
        if (keycode & 0x80)
        {
            KB_DealKeyUp(keycode & 0x7F);
        }
        else
        {
            KB_DealKeyDown(keycode & 0x7F);
        }
    }
}
__attribute__((interrupt)) void int0(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x00\r\n");
}

__attribute__((interrupt)) void int1(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x01\r\n");
}
__attribute__((interrupt)) void int2(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x02\r\n");
}
__attribute__((interrupt)) void int3(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x03\r\n");
}
__attribute__((interrupt)) void int4(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x04\r\n");
}
__attribute__((interrupt)) void int5(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x05\r\n");
}
__attribute__((interrupt)) void int6(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x06");
}
__attribute__((interrupt)) void int7(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x07\r\n");
}
__attribute__((interrupt)) void int8(struct interrupt_frame *frame, unsigned int error_code)
{
    Puts("Interrupt: 0x08\r\n");
}
__attribute__((interrupt)) void int9(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x09\r\n");
}
__attribute__((interrupt)) void int10(struct interrupt_frame *frame, unsigned int error_code)
{
    Puts("Interrupt: 0x0A\r\n");
}

// 编写中断号11-20的函数类似，只需修改相应的ASCII码和中断号

__attribute__((interrupt)) void int11(struct interrupt_frame *frame, unsigned int error_code)
{
    Puts("Interrupt: 0x0B\r\n");
}

// 中断号12-20的函数依次类推

// 中断号12
__attribute__((interrupt)) void int12(struct interrupt_frame *frame, unsigned int error_code)
{
    Puts("Interrupt: 0x0C\r\n");
}
__attribute__((interrupt)) void int13(struct interrupt_frame *frame, unsigned int error_code)
{
    Puts("Interrupt: 0x0D\r\n");
    PrintHex(frame->eip);
    WaitForInterrupt();
}

__attribute__((interrupt)) void int14(struct interrupt_frame *frame, unsigned int error_code)
{

    unsigned int lineraddr;
    asm volatile(
        "mov %%cr2,%%eax\n\t"
        : "=a"(lineraddr)
        :);
    char str[256] = "Page error at 0x";
    char temp[9] = {0};
    Strcat(str, UIntToHex(frame->eip, temp));
    Strcat(str, " Error lineraddr is 0x");
    Strcat(str, UIntToHex(lineraddr, temp));
    Strcat(str, "\r\n");
    Puts(str);
    CloseInterrupt();
    WaitForInterrupt();
    Puts("Press any key to continue...\r\n");
    SetEFLAGS(frame->eflags);
    WaitForKeyboardInput();
    CloseNowTask();
}

__attribute__((interrupt)) void int15(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x0F\r\n");
}

__attribute__((interrupt)) void int16(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x10\r\n");
}

__attribute__((interrupt)) void int17(struct interrupt_frame *frame, unsigned int error_code)
{
    Puts("Interrupt: 0x11\r\n");
}
__attribute__((interrupt)) void int18(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x12\r\n");
}

__attribute__((interrupt)) void int19(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x13\r\n");
}
__attribute__((interrupt)) void int20(struct interrupt_frame *frame)
{
    Puts("Interrupt: 0x14\r\n");
}
__attribute__((interrupt)) volatile void int0x66(struct interrupt_frame *frame)
{
    unsigned int int_num, b, c, d, esi, edi;
    asm volatile(
        ""
        : "=D"(int_num), "=b"(b), "=c"(c), "=d"(d), "=S"(esi)
        :);
    deal_soft_interrupt(int_num, b, c, d);
}

__attribute__((interrupt)) void int0x21(struct interrupt_frame *frame)
{
    WritePort(0xa0, 0x20);
    WritePort(0x20, 0x20);
    DealKeyboartInterrupt();
}

__attribute__((interrupt)) void int0x70(struct interrupt_frame *frame)
{
    WritePort(0xA0, 0x20);
    WritePort(0x20, 0x20);
    WritePort(0x70, 0x0C);
    ReadPort(0x71);
    asm volatile(
        ""
        :
        :
        : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    switch_task();
}

void int_default(struct interrupt_frame *frame)
{
    WritePort(0xA0, 0x20);
    WritePort(0x20, 0x20);
}
