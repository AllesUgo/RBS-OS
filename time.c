#include "time.h"
#include "base_io.h"
void EnableTimeInterrupt()
{
    unsigned int eflags=GetEFLAGS();
    CloseInterrupt();
    WritePort(0x70,0x0B|0x80);
    WritePort(0x71,0x42);
    WritePort(0x70,0x0A);
    WritePort(0x71,0x27);
    
    char a=ReadPort(0xA1);
    a&=0xFE;
    WritePort(0xA1,a);
    WritePort(0x70,0x0C);
    ReadPort(0x71);
    SetEFLAGS(eflags);
}