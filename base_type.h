#pragma once
#define NULL 0x0
struct DESCRIPTOR
{
    unsigned int low;
    unsigned int height;
} __attribute__((packed));
struct DESCRIPTOR_REG
{
    unsigned short int segment_limit;
    unsigned int base_addr;
} __attribute__((packed));

struct TASK_EXTRA_INFO {
    int last_input_keyscan_code;
    struct Session* session;
};

struct TASK_INFO_BLOCK
{
    unsigned int eax;//0
    unsigned int ebx;//4
    unsigned int ecx;//8
    unsigned int edx;//12
    unsigned int esi;//16
    unsigned int edi;//20
    unsigned int esp;//24
    unsigned int ebp;//28
    unsigned int EFLAGS;//32
    unsigned int cr3;//36
    unsigned int eip;//40
    unsigned short int gs;//44
    unsigned short int fs;//46
    unsigned short int es;//48
    unsigned short int ds;//50
    unsigned short int ss;//52
    unsigned short int cs;//54
    unsigned int esp0;//56
    unsigned int next_allocate;//60
    unsigned int task_state;//0空闲 1执行中 3暂停 4等待回收 64
    unsigned int wait_to_time;
    int is_wait_key;
    struct TASK_INFO_BLOCK*next;
    unsigned int task_id;
    void* extra_info;
    struct TASK_EXTRA_INFO _extra;
    char task_name[0];
} __attribute__((packed));

struct TSS
{
    unsigned int front_tss;
    unsigned int ESP0;
    unsigned int SS0;
    unsigned int ESP1;
    unsigned int SS1;
    unsigned int ESP2;
    unsigned int SS2;
    unsigned int CR3;
    unsigned int EIP;
    unsigned int EFLAGS;
    unsigned int EAX;
    unsigned int ECX;
    unsigned int EDX;
    unsigned int EBX;
    unsigned int ESP;
    unsigned int EBP;
    unsigned int ESI;
    unsigned int EDI;
    unsigned int ES;
    unsigned int CS;
    unsigned int SS;
    unsigned int DS;
    unsigned int FS;
    unsigned int GS;
    unsigned int LDT;
    unsigned short int T;
    unsigned short int IO_BASE;
} __attribute__((packed));

struct interrupt_frame
{
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;
} __attribute__((packed));