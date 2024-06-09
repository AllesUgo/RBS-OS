#pragma once

void SetCursor(int x,int y);
void GetCursor(int*x,int*y);
void CleanScreen(void);
void Putchar(unsigned char ch,unsigned char attribute);
void Puts(const char*str);
void PrintHex(unsigned int num);
void PrintNumber(int number,int radix);
