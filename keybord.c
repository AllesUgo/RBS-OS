#include "keybord.h"
#include "base_io.h"
#include "task.h"
#include "session.h"
static char KEY_STATE_MAP[256] = {0};
void EnableKeybordInterrupt()
{
    unsigned int eflags=GetEFLAGS();
    CloseInterrupt();
    // 读取当前控制器状态
    //WritePort(0x64, 0x20);
    //unsigned char status = ReadPort(0x60);

    // 修改控制器状态启用键盘中断
    //status |= 0x01;

    // 写回修改后的控制器状态
    //WritePort(0x64, 0x60);
    //WritePort(0x60, status);
    //设置8259A的IR1位非屏蔽
    unsigned int status=ReadPort(0x21);
    status&=0xFD;
    WritePort(0x21,status);
    // 启用键盘中断
    //WritePort(0x64, 0xAD);//启用键盘
    SetEFLAGS(eflags);
}
//本函数不考虑中断，默认中断已关闭
void KB_DealKeyDown(unsigned char scan_code)
{
    KEY_STATE_MAP[scan_code] = 1;
    //int x = MapScanCodeToChar(scan_code);
    //if (x) Session_KeyDown(x);
    /*
    struct Session* session = Session_GetNowSession();
    if (session)
    {
        Session_PrintNumber(session,scan_code,10);
        if (scan_code == KEY_F1) Session_PutString(session,"F1\r\n");
    }
    */
   //检查当前进附加的进程是否等待接收
    //struct Session* session = Session_GetNowSession();
    int tid = Session_GetNowAttachedTask();
    if (tid<0) return;
    struct TASK_INFO_BLOCK* info = GetTaskInfo(tid);
    if (info==(void*)0xFFFFFFFF)
    {
        //异常崩溃
        CloseInterrupt();
        struct Session* session = Session_GetNowSession();
        Session_PutString(session,"\r\nHLT");
        WaitForInterrupt();
    }
    //检查目标进程是否在等待输入
    if (info->is_wait_key) 
    {
        ((struct TASK_EXTRA_INFO*)info->extra_info)->last_input_keyscan_code = scan_code;
        //恢复目标进程执行
        WakeUpTask(tid);
    }
}
void KB_DealKeyUp(unsigned char scan_code)
{
    KEY_STATE_MAP[scan_code] = 0;
}
int MapScanCodeToChar(unsigned char scan_code)
{
    static short chars_lower[90] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                                    '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
                                    0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
                                    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0};
    static short chars_upper[90] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
                                    '_', '+', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
                                    0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',
                                    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' ', 0};
    if (scan_code > 90)
        return 0;
    if (KEY_STATE_MAP[KEY_LEFT_SHIFT] == 1)
        return chars_upper[scan_code];
    else
        return chars_lower[scan_code];
}

int KB_GetKeyStatus(unsigned int scan_code)
{
    if (scan_code>255) return 0;
    return KEY_STATE_MAP[scan_code];
}
