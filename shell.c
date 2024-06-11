#include "shell.h"
#include "task.h"

char* Getline(char*buffer,int max_len)
{
    int i=0;
    max_len-=1;
    while (i<max_len)
    {
        int key = WaitForKeyboardInput();
        if (key==KEY_ENTER) break;
        char ch = MapScanCodeToChar(key);
        if (ch==0) continue;
        else buffer[i++] = ch;
    }
    buffer[i] = 0;
    return buffer;
}

char GetChar()
{

}

char GetCh()
{
    char ch = WaitForKeyboardInput();
    
}
