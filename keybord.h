#pragma once 

// Special keys macro definitions
#define KEY_ESC     0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB     0x0F
#define KEY_ENTER   0x1C
#define KEY_CTRL    0x1D
#define KEY_LEFT_SHIFT  0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_ALT     0x38
#define KEY_SPACE   0x39
#define KEY_CAPS_LOCK   0x3A
#define KEY_F1      0x3B
#define KEY_F2      0x3C
#define KEY_F3      0x3D
#define KEY_F4      0x3E
#define KEY_F5      0x3F
#define KEY_F6      0x40
#define KEY_F7      0x41
#define KEY_F8      0x42
#define KEY_F9      0x43
#define KEY_F10     0x44
#define KEY_F11     87
#define KEY_F12     88
#define KEY_NUM_LOCK    0x45
#define KEY_SCROLL_LOCK 0x46
#define KEY_HOME    0x47
#define KEY_UP_ARROW    0x48
#define KEY_PAGE_UP     0x49
#define KEY_MINUS   0x4A
#define KEY_LEFT_ARROW  0x4B
#define KEY_CENTER  0x4C
void EnableKeybordInterrupt();
void KB_DealKeyDown(unsigned char scan_code);
void KB_DealKeyUp(unsigned char scan_code);
int MapScanCodeToChar(unsigned char scan_code);
int KB_GetKeyStatus(unsigned int scan_code);

