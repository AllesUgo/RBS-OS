#pragma once

struct BUFFER
{
    char *buffer;
    unsigned int buffer_size;
    char *start;
    char *end;
}; /*利用循环队列*/

void Buffer_CreateBuffer(struct BUFFER *buffer, char *buffer_memory_ptr, unsigned int buffer_size);
int Buffer_Write(struct BUFFER *buffer, const void *content, int content_length);
int Buffer_Read(struct BUFFER *buffer, void *ptr, int length);
unsigned int GetBufferUsedSize(struct BUFFER *buffer); // 计算空间剩余量
