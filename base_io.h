#pragma once
#ifndef BASE_IO_H
#define BASE_IO_H
unsigned int GetEFLAGS(void);
void SetEFLAGS(unsigned int eflags);
void CloseInterrupt();
void StartInterrupt();
unsigned char ReadPort(unsigned short int Port);
void WritePort(unsigned short int port, unsigned char data);
void WaitForInterrupt();
void ReadSector(void *buffer, unsigned int sector_id, unsigned char need_num);
void FlushCR3(void);
void CloseCPU();

#endif
