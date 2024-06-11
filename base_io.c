#include "base_io.h"
#include "screen_ctrl.h"
unsigned int GetEFLAGS(void)
{
  unsigned int r;
  asm volatile(
      "pushf\n\t"
      "pop %%eax\n\t"
      : "=a"(r)
      :
      );
  return r;
}
void SetEFLAGS(unsigned int eflags)
{
  asm volatile(
      "push %%eax\n\t"
      "popf\n\t"
      :
      : "a"(eflags)
      );
}
void CloseInterrupt()
{
  asm volatile("cli\n\t");
}
void StartInterrupt()
{
  asm volatile("sti\n\t");
}

unsigned char ReadPort(unsigned short int Port)
{
  unsigned char res;
  asm volatile("in %%dx,%%al\n\t"
               : "=a"(res)
               : "d"(Port));
  return res;
}
void WritePort(unsigned short int port, unsigned char data)
{
  asm volatile("out %%al,%%dx"
               :
               : "a"(data), "d"(port));
}

void WaitForInterrupt()
{
  asm volatile("hlt\n\t");
}
void ReadSector(void *buffer, unsigned int sector_id, unsigned char need_num)
{
  unsigned char state;
  WritePort(0x1F2, need_num);
  WritePort(0x1F3, sector_id & 0xFF);
  WritePort(0x1F4, (sector_id & 0xFF00) >> 8);
  WritePort(0x1F5, (sector_id & 0xFF0000) >> 16);
  WritePort(0x1F6, ((sector_id & 0xF000000) >> 24) | 0xE0);
  WritePort(0x1F7, 0x20); // 请求硬盘读
  //asm volatile ("xchg %bx,%bx");
  do
  {
    state = ReadPort(0x1F7);

  } while ((state & 0x88) != 0x08);
  //asm volatile ("xchg %bx,%bx");
  unsigned short int *ptr = (unsigned short int *)buffer;
  for (unsigned int i = 0; i < need_num * 256; i++)
  {
    asm volatile(
        "mov $0x1F0,%%dx\n\t"
        "in %%dx,%%ax\n\t"
        "mov %%ax,(%%ecx)\n\t"
        :
        : "c"(ptr)
        :"%edx","%eax");
    ptr += 1;
  }
  //asm volatile ("xchg %bx,%bx");
}

void FlushCR3(void)
{
  asm volatile(
      "push %%eax\r\n"
      "mov %%cr3,%%eax\r\n"
      "mov %%eax,%%cr3\r\n"
      "pop %%eax\r\n"
      :
      :);
}
void CloseCPU()
{
  __asm__ volatile("cli;hlt;");
}

