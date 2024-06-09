#include "screen_ctrl.h"
#include "base_io.h"
#include "basic_info.h"
#include "task.h"
#include "c_basic.h"

void SetCursor(int x,int y)
{
    unsigned short int cursor=y*80+x;
    WritePort(0x3d4,14);
    WritePort(0x3d5,cursor>>8);
    WritePort(0x3d4,15);
    WritePort(0x3d5,cursor&0x00ff);
}
void GetCursor(int*x,int*y)
{
    unsigned char height,low;
    WritePort(0x3d4,14);
    height=ReadPort(0x3d5);
    WritePort(0x3d4,15);
    low=ReadPort(0x3d5);
    unsigned short int cursor;
    cursor=height;
    cursor<<=8;
    cursor|=(unsigned short int)low;
    *x=cursor%80;
    *y=cursor/80;
}
void CleanScreen(void)
{
    char *p=(char*)TextViewAddress;
    for (int i=0;i<2000;i+=1) p[i*2]=' ',p[i*2+1]=0x07;
    SetCursor(0,0);
}
void Putchar(unsigned char ch,unsigned char attribute)
{
    DisableSwitchTask();
    unsigned int eflags=GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    int x,y;
    char *p=(char*)TextViewAddress;                                                                                                                
    GetCursor(&x,&y);
    switch (ch)
    {
    case '\n':
        y+=1;
        break;
    case '\r':
        x^=x;
        break;
    default:
        p[(y*80+x)*2]=ch;
        p[(y*80+x)*2+1]=attribute;
        ++x;
        break;
    }
    if (x>=80) ++y,x=0;
    if (y>=25)
    {
        //向上滚屏
        for (int i=0;i<1980*2;++i)
        {
            p[i]=p[i+80*2];
        }
        for (int i=1920*2;i<2000*2;) p[i++] = ' ',p[i++] = 0x07;
        --y;
    }
    SetCursor(x,y);
    SetEFLAGS(eflags);
}
void Puts(const char*str)
{
    DisableSwitchTask();
    unsigned int eflags=GetEFLAGS();
    CloseInterrupt();
    for (int i=0;str[i];++i) Putchar(str[i],0x07);
    RecoverSwitchTask();
    SetEFLAGS(eflags);
}

void PrintHex(unsigned int num)
{
    DisableSwitchTask();
    unsigned int eflags=GetEFLAGS();
    CloseInterrupt();
    RecoverSwitchTask();
    char arr[]="0123456789ABCDEF";
    char str[8]={0};
    for (int i=0;i<8;++i)
    {
        str[i]=arr[num%16];
        num/=16;
    }
    for (int i=7;i>=0;--i)
    {
        if (str[i]==0) Putchar('0',0x07);
        else Putchar(str[i],0x07);
    }
    Putchar('\r',0x07);
    Putchar('\n',0x07);
    SetEFLAGS(eflags);
}
//打印整数，radix为进制
void PrintNumber(int number,int radix)
{
	char temp[20]={0};
    Puts(Itoa(number,temp,radix));
    Puts("\r\n");
}
