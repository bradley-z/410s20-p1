/** @file kb_buffer.c
 *  @brief keyboard buffer implementation
 *
 *  Implementation for the keyboard buffer defined in kb_buffer.h. This is a
 *  pretty simple and standard circular buffer.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug described in kb_buffer.h
 */
#include <kb_buffer.h>

void kb_buf_initialize(kb_buf_t *kb_buf)
{
    /* initially, a circular buffer starts out with read/write index as 0 */
    kb_buf->read_index = 0;
    kb_buf->write_index = 0;
}

bool kb_buf_read(kb_buf_t *kb_buf, int *read_result)
{
    int read_index = kb_buf->read_index;
    int write_index = kb_buf->write_index;

    /* a circ buffer is empty when the read_index equals write_index */
    if (read_index == write_index) {
        return false;
    }

    /**
     *  Read value before incrementing read_index because otherwise, a write
     *  in between the increment and the read could mistakenly classify the
     *  buffer as full when it is not, though this case is not nearly as bad as
     *  in kb_buf_write. In this case, we might just drop slightly more keypress
     *  events than necessary.
     */
    *read_result = kb_buf->keypress_queue[read_index];
    kb_buf->read_index = (read_index + 1) % CIRCULAR_BUFFER_SIZE;
    return true;
}

bool kb_buf_write(kb_buf_t *kb_buf, int keypress)
{
    int read_index = kb_buf->read_index;
    int write_index = kb_buf->write_index;

    /* a circ buffer is full when the read_index is 1 less than write_index */
    if ((read_index + 1) % CIRCULAR_BUFFER_SIZE == write_index) {
        /* just drop keypress event if buffer is full */
        return false;
    }

    /**
     *  We want to write the scancode into the buffer THEN increment the
     *  write_index. If we don't, a situation could arise where we write into an 
     *  empty buffer, increment write_index, and then get a read call, which
     *  gets the incorrect value from the buffer as the correct value has not
     *  been written there yet.
     */
    kb_buf->keypress_queue[write_index] = keypress;
    kb_buf->write_index = (write_index + 1) % CIRCULAR_BUFFER_SIZE;
    return true;
}
