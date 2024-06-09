#include "buffer.h"

void Buffer_CreateBuffer(struct BUFFER *buffer, char *buffer_memory_ptr, unsigned int buffer_size)
{
    buffer->buffer = buffer_memory_ptr;
    buffer->buffer_size = buffer_size;
    buffer->start = buffer->end = buffer_memory_ptr;
}

unsigned int GetBufferUsedSize(struct BUFFER *buffer)
{
    // 计算空间剩余量
    if (buffer->end >= buffer->start)
        return buffer->end - buffer->start;
    else
        return (buffer->end - buffer->buffer) + (buffer->buffer + buffer->buffer_size - buffer->start);
}

int Buffer_Write(struct BUFFER *buffer, const void *content, int content_length)
{
    if (buffer->buffer_size - GetBufferUsedSize(buffer) <= content_length)
        return -1;
    const char *p = (const char *)content;
    const char *end = (const char *)buffer->buffer + buffer->buffer_size;
    for (int i = 0; i < content_length; ++i, ++p, ++buffer->end)
    {
        if (buffer->end >= end)
            buffer->end = buffer->buffer;
        *buffer->end = *p;
    }
    if (buffer->end >= end)
        buffer->end = buffer->buffer;
    return 0;
}

int Buffer_Read(struct BUFFER *buffer, void *ptr, int length)
{
    
}
