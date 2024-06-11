#include "session.h"
#include "c_basic.h"
#include "memory.h"
#include "screen_ctrl.h"
#include "basic_info.h"
static struct Session *ATTACHED_SESSION = 0x00;
static void FlushSessionToScreen(struct Session *session)
{
    unsigned int *d = (unsigned int *)TextViewAddress;
    unsigned int *s = (unsigned int *)session->text;
    unsigned int count = sizeof(session->text) / sizeof(unsigned int);
    while (count--)
        *(d++) = *(s++);
    SetCursor(session->cursor_x, session->cursor_y);
}
static inline void PutcharWhithNotFlush(struct Session *session, char ch, unsigned char attribute)
{
    int x = session->cursor_x, y = session->cursor_y;
    char *p = session->text;
    switch (ch)
    {
    case '\n':
        y += 1;
        break;
    case '\r':
        x = 0;
        break;
    default:
        p[(y * 80 + x) * 2] = ch;
        p[(y * 80 + x) * 2 + 1] = attribute;
        ++x;
        break;
    }
    if (x >= 80)
        ++y, x = 0;
    if (y >= 25)
    {
        
        // 向上滚屏
        for (int i = 0; i < 1920 * 2; ++i)
        {
            //asm volatile("xor %ax,%ax\n\t");
            p[i] = p[i + 80 * 2];
        }
        for (int i=1920*2;i<2000*2;) p[i++] = ' ',p[i++] = 0x07;
        
        --y;
    }
    session->cursor_x = x;
    session->cursor_y = y;
}
struct Session *Session_CreateSession(void)
{
    struct Session *p = kmalloc(sizeof(struct Session));
    p->cursor_x = p->cursor_y = p->is_attached = p->session_id = 0;
    p->scan_codes_end = p->scan_codes_front = 0;
    for (int i = 0; i < 2000; ++i)
    {
        p->text[i * 2] = ' ';
        p->text[i * 2 + 1] = 0x07;
    }
    for (int i = 0; i < SESSION_MAX_ATTACHD_TASK_NUM; ++i)
        p->tasks[i] = -1;
    p->attached_task_front = -1;
    return p;
}

void Session_ReleaseSession(struct Session *session)
{
    if (ATTACHED_SESSION == session)
        ATTACHED_SESSION = 0x00;
    session->is_attached = 0;
    kfree(session);
}

void Session_Putchar(struct Session *session, char ch, unsigned char attribute)
{
    PutcharWhithNotFlush(session, ch, attribute);
    if (ATTACHED_SESSION == session)
        FlushSessionToScreen(ATTACHED_SESSION);
}

void Session_SetCursor(struct Session *session, short int x, short int y)
{
    if (x<0||x>80||y<0||y>25) return;
    session->cursor_x = x;
    session->cursor_y = y;
    if (ATTACHED_SESSION == session)
        FlushSessionToScreen(ATTACHED_SESSION);
}

void Session_GetCursor(struct Session *session, short int *x, short int *y)
{
    *x = session->cursor_x;
    *y = session->cursor_y;
}

void Session_SwitchToSession(struct Session *target_session)
{
    ATTACHED_SESSION = target_session;
    FlushSessionToScreen(ATTACHED_SESSION);
}

void Session_PutString(struct Session *session, const char *str)
{
    for (; *str; ++str)
    {
        PutcharWhithNotFlush(session, *str, 0x07);
    }
    if (ATTACHED_SESSION == session)
        FlushSessionToScreen(ATTACHED_SESSION);
}

void Session_KeyDown(unsigned char key_scan_code)
{
    if (ATTACHED_SESSION)
    {
        if ((ATTACHED_SESSION->scan_codes_end + 1) % SESSION_SCAN_CODES_BUFFER_SIZE == ATTACHED_SESSION->scan_codes_front)
            return;
        else
        {
            ATTACHED_SESSION->scan_codes[ATTACHED_SESSION->scan_codes_end] = key_scan_code;
            ATTACHED_SESSION->scan_codes_end = (ATTACHED_SESSION->scan_codes_end + 1) % SESSION_SCAN_CODES_BUFFER_SIZE;
        }
        // 检查是否要显示在屏幕上
        if (key_scan_code == 0x1c)
        {
            Session_PutString(ATTACHED_SESSION, "\r\n");
        }
        else
            Session_Putchar(ATTACHED_SESSION, key_scan_code, 0x07);
    }
}

int Session_AttachTask(struct Session *session, int task_id)
{
    if (session->attached_task_front + 1 >= SESSION_MAX_ATTACHD_TASK_NUM)
        return -1;
    session->tasks[++(session->attached_task_front)] = task_id;
    return 0;
}
/*从指定session上移除附加的task,成功返回0，失败返回-1*/
int Session_DetachTask(struct Session *session,int tid)
{
    for (int i=0;i<=session->attached_task_front;++i)
    {
        if (session->tasks[i]==tid)
        {
            //分离该进程，将后面的前移
            for (int j=i+1;j<=session->attached_task_front;++j)
            {
                session->tasks[j-1] = session->tasks[j];
            }
            session->attached_task_front-=1;
            return 0;
        }
    }
    return -1;
}

int Session_GetNowAttachedTask(void)
{
    if (ATTACHED_SESSION == 0x00) return -1;
    if (ATTACHED_SESSION->attached_task_front <0) return -2;
    return ATTACHED_SESSION->tasks[ATTACHED_SESSION->attached_task_front];
}
//以指定进制打印整数在Session上
void Session_PrintNumber(struct Session* Session,int num,int a)
{
    char temp[20]={0};
    Session_PutString(Session,Itoa(num,temp,a));
    Session_PutString(Session,"\r\n");
}
//获取当前展示的session
struct Session* Session_GetNowSession(void)
{
    return ATTACHED_SESSION;
}
