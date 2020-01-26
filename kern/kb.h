#ifndef __KB_H
#define __KB_H

#include <p1kern.h>

#define TOTAL_KEYS 128
#define CIRCULAR_BUFFER_SIZE (TOTAL_KEYS * 2)

int keypresses[CIRCULAR_BUFFER_SIZE];
unsigned int read_index = 0;
unsigned int write_index = 0;

#endif /* __KB_H */