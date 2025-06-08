#include <sys_defs.h>
#include <buffer.h>

int8_t enqueue_byte(stream_buffer *buf, uint8_t b)
{
    if (buf == NULL || buf->idx >= 128)
    {
        return -1;
    }

    buf->buffer[buf->idx++] = b;
    return 0;
}

void init_buffer(stream_buffer *buf)
{
    buf->idx = 0;
    buf->ready = 0;
}

int8_t copy_buffer(stream_buffer *source, stream_buffer *dest);

void reset_buffer(stream_buffer *buf)
{
    memset(buf->buffer, 0, 128);
    buf->idx = 0;
    buf->ready = false;
}

