#pragma once
#define SESSION_TEXT_BUFFER_SIZE
#define SESSION_SCAN_CODES_BUFFER_SIZE 1024
#define SESSION_MAX_ATTACHD_TASK_NUM 100
struct Session
{
    int session_id, is_attached;
    int cursor_x, cursor_y;
    unsigned int scan_codes_front,scan_codes_end;//有效区域为从front到end的前一个
    int attached_task_front;
    char text[4000];
    unsigned char scan_codes[SESSION_SCAN_CODES_BUFFER_SIZE];
    int tasks[SESSION_MAX_ATTACHD_TASK_NUM];
};
struct Session *Session_CreateSession(void);
void Session_ReleaseSession(struct Session* session);
void Session_Putchar(struct Session *session, char ch, unsigned char attribute);
void Session_SetCursor(struct Session *session, short int x, short int y);
void Session_GetCursor(struct Session *session, short int *x, short int *y);
void Session_SwitchToSession(struct Session *target_session); 
void Session_PutString(struct Session *Session,const char* str);
void Session_KeyDown(unsigned char key_scan_code);
int Session_AttachTask(struct Session*session,int task_id);
int Session_DetachTask(struct Session*session,int task_id);
int Session_GetNowAttachedTask(void);//返回-1表示当前没有正在显示的Session，-2表示没有已经附加的进程
void Session_PrintNumber(struct Session* Session,int num,int a);
struct Session* Session_GetNowSession(void);//获取当前展示的session
