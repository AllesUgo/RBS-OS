#pragma once 
int Strlen(const char* str);
void Strcpy(char* dst, const char* src);
char* Strcat(char* dst, const char* src);
void Memset(void* ptr, int set_num, int buffer_len);
void Memcpy(void* dst,const void* src, int buffer_len);
char* Itoa(int num, char* str, int radix);
void Sprintf(char* buf, const char* fmt, ...);
char* UIntToHex(unsigned int x,char*str);
