#pragma once

#include "base_type.h"
#include "task_basic_info.h"
#include "session.h"



unsigned int AllocateTaskID(void);
void CleanTask(unsigned int task_id);

struct DESCRIPTOR MakeSegmentDescriptor(void *base_addr, unsigned int segment_limit, unsigned short int type);
struct DESCRIPTOR MakeGateDescriptor(unsigned int offset, unsigned short int seg, int p, int dpl, int type, int par_num);

unsigned short int InstallGDT(struct DESCRIPTOR des, struct DESCRIPTOR_REG *addr);

void FlashGDT(struct DESCRIPTOR_REG *addr);

void InitKernelTask();

void switch_task(void);
int ReadProgramSize(unsigned int sector);
int ReadProgram(char *buffer, unsigned int buffer_size, unsigned int sector);

int LoadTask(int sector, int dpl,struct Session* session);

unsigned int AllocateTaskID(void);

void CleanTask(unsigned int task_id);

void CloseNowTask(void);
int PauseTask(unsigned int task_id);//return 0xFFFFFFFFis failed
struct TASK_INFO_BLOCK* GetTaskInfo(unsigned int task_id);//本函数不关终端部，需要在调用前确保不会发生任务链表修改和任务链表完整 return 0xFFFFFFFF is failed
struct TASK_INFO_BLOCK* GetNowTaskInfoPtr(void);
void SetNowTaskInfoPtr(struct TASK_INFO_BLOCK* ptr);
void DisableSwitchTask();
void RecoverSwitchTask();

void SleepNowTask(unsigned int time_ms);
void WakeUpTask(unsigned int task_id);

int WaitForKeyboardInput();
void SwitchToNextTask();
//统计当前任务个数，本函数不保证进程安全
int GetTaskNumber();

void* TaskHeapUp(unsigned int size);
void TaskHeapDown(unsigned int size);

