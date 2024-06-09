#pragma once
#include "base_type.h"
// 函数定义
int InstallInterrupt(int interrupt_id, struct DESCRIPTOR des, struct DESCRIPTOR_REG* addr);
void FlashInterrupt(struct DESCRIPTOR_REG* addr);
void Init8259A();
void __cdecl deal_soft_interrupt(unsigned int num,unsigned int b,unsigned int c,unsigned int d);
void DealKeyboartInterrupt(void);
__attribute__((interrupt))
void int0(struct interrupt_frame*frame);

__attribute__((interrupt))
void int1(struct interrupt_frame*frame);

__attribute__((interrupt))
void int2(struct interrupt_frame*frame);

__attribute__((interrupt))
void int3(struct interrupt_frame*frame);

void int4();
__attribute__((interrupt))

__attribute__((interrupt))
void int5(struct interrupt_frame*frame);

__attribute__((interrupt))
void int6(struct interrupt_frame*frame);

__attribute__((interrupt))
void int7(struct interrupt_frame*frame);

__attribute__((interrupt))
void int8(struct interrupt_frame*frame,unsigned int error_code);

__attribute__((interrupt))
void int9(struct interrupt_frame*frame);

__attribute__((interrupt))
void int10(struct interrupt_frame*frame,unsigned int error_code);

// 编写中断号11-20的函数类似，只需修改相应的ASCII码和中断号
__attribute__((interrupt))
void int11(struct interrupt_frame*frame,unsigned int error_code);
// 中断号12-20的函数依次类推
// 中断号12
__attribute__((interrupt))
void int12(struct interrupt_frame*frame,unsigned int error_code);

__attribute__((interrupt))
void int13(struct interrupt_frame*frame,unsigned int error_code);

__attribute__((interrupt))
void int14(struct interrupt_frame*frame,unsigned int error_code);

__attribute__((interrupt))
void int15(struct interrupt_frame*frame);

__attribute__((interrupt))
void int16(struct interrupt_frame*frame);

__attribute__((interrupt))
void int17(struct interrupt_frame*frame,unsigned int error_code);

__attribute__((interrupt))
void int18(struct interrupt_frame*frame);

__attribute__((interrupt))
void int19(struct interrupt_frame*frame);

__attribute__((interrupt))
void int20(struct interrupt_frame*frame);

__attribute__((interrupt)) 
void volatile int0x66(struct interrupt_frame*frame);

__attribute__((interrupt))
void int0x21(struct interrupt_frame* frame);

__attribute__((interrupt))
void int0x70(struct interrupt_frame* frame);

__attribute__((interrupt))
void int_default(struct interrupt_frame* frame);


