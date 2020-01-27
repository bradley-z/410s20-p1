#ifndef __KB_BUFFER_H
#define __KB_BUFFER_H

#include <stdbool.h>

#define TOTAL_KEYS 128
// this is basically enough for every key to be pressed and released once
#define CIRCULAR_BUFFER_SIZE (TOTAL_KEYS * 2)

typedef struct {
    int keypress_queue[CIRCULAR_BUFFER_SIZE];
    int read_index;
    int write_index;
} kb_buf_t;

void kb_buf_clear(kb_buf_t *kb_buf);
bool kb_buf_read(kb_buf_t *kb_buf, int *read_result);
bool kb_buf_write(kb_buf_t *kb_buf, int keypress);

#endif /* __KB_BUFFER_H */
