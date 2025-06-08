#include <sys_defs.h>

typedef struct
{
    volatile bool ready;
    volatile size_t idx;
    uint8_t buffer[128];
} stream_buffer;

int8_t enqueue_byte(stream_buffer *buf, uint8_t b);
int8_t copy_buffer(stream_buffer *source, stream_buffer *dest);
void reset_buffer(stream_buffer *buf);
void init_buffer(stream_buffer *buf);
