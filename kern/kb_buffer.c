#include <kb_buffer.h>

void kb_buf_initialize(kb_buf_t *kb_buf)
{
    kb_buf->read_index = 0;
    kb_buf->write_index = 0;
}

bool kb_buf_read(kb_buf_t *kb_buf, int *read_result)
{
    int read_index = kb_buf->read_index;
    int write_index = kb_buf->write_index;

    if (read_index == write_index) {
        return false;
    }

    *read_result = kb_buf->keypress_queue[read_index];
    kb_buf->read_index = (read_index + 1) % CIRCULAR_BUFFER_SIZE;
    return true;
}

bool kb_buf_write(kb_buf_t *kb_buf, int keypress)
{
    int read_index = kb_buf->read_index;
    int write_index = kb_buf->write_index;

    if ((read_index + 1) % CIRCULAR_BUFFER_SIZE == write_index) {
        return false;
    }

    kb_buf->keypress_queue[write_index] = keypress;
    kb_buf->write_index = (write_index + 1) % CIRCULAR_BUFFER_SIZE;
    return true;
}
