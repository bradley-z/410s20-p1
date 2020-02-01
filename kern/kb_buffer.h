/** @file kb_buffer.h
 *  @brief keyboard buffer interface
 *
 *  This is the interface for the implementation of how keypress events are
 *  queued. The approach I chose was to use a circular buffer to keep track of
 *  such keypress events. I chose this based on the necessity of having FIFO
 *  functionality while also not having the potential overhead of having an
 *  unbounded size in the queue. If we compare cases where keypress events
 *  aren't being processed, dropping additional keypress events is preferable
 *  compared to the queue growing potentially indefinitely.
 *
 *  Because processing the actual keypress scancode is a potentially costly
 *  operation and some of these functions are to be called from the keyboard
 *  interrupt handler, the raw, unprocessed scancodes are stored in this buffer.
 *  
 *  Circular buffers are thread safe in a single-producer single-consumer
 *  scenario, which is what we have. The producer is the interrupt handler upon
 *  a keypress event and the consumer is the keyboard driver. This is thread
 *  safe because the producer and consumer are acting on two different parts of
 *  the buffer (read_index/write_index) and the only case where they overlap is
 *  if the queue is empty (read_index == write_index). In this case, if both
 *  are accessed simultaneously, then the read will just return -1 and wait
 *  for the write to complete.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug The underlying buffer storing keypresses is a fixed size. Therefore, if
 *       a situation arises where calls to kb_buf_write() are occurring at a
 *       much faster rate than kb_buf_read(), the underlying buffer could fill
 *       up. If the buffer is full, then the keypress scancode is just dropped.
 *       This could lead to unintended behaviors such as a press event not
 *       having an associated release event or a modifier key (shift/ctrl/etc)
 *       modifying an unintended key.
 */
#ifndef __KB_BUFFER_H_
#define __KB_BUFFER_H_

#include <stdbool.h>    /* bool */

/**
 *  keyhelp.h defines 127 possible keys in the kh_extended_e enum, round up for
 *  nice power of two
 */
#define TOTAL_KEYS 128
/**
 *  The logic behind this choice was to be able to store one press/release event
 *  for each possible key. Doesn't account the possiblity of pressing and
 *  releasing the same key multiple times without being processed, leading to
 *  the potential buf as described in the top.
 */
#define CIRCULAR_BUFFER_SIZE (TOTAL_KEYS * 2)

/* circular buffer struct */
typedef struct {
    int keypress_queue[CIRCULAR_BUFFER_SIZE];
    int read_index;
    int write_index;
} kb_buf_t;

/** @brief initialize the keyboard buffer
 * 
 *  C doesn't have elegant default constructors :( so this function just zero's
 *  out read_index and write_index of the buffer passed in, which is the state
 *  a circular buffer should be at first.
 *
 *  @param kb_buf pointer to buffer to initialize
 *  @return Void.
 */
void kb_buf_initialize(kb_buf_t *kb_buf);
/** @brief reads the next keypress scancode from the keyboard buffer
 * 
 *  If the queue is empty, false is returned. Otherwise, read the next keypress
 *  scancode from the queue, store it into read_result, and return true. The
 *  read_index is then incremented, wrapping back to 0 if necessary.
 *  
 *  @param kb_buf pointer to keyboard buffer to try to read next scancode from
 *  @param read_result pointer to int to store scancode at if buffer is not full
 *  @return whether or not the read was successful (if the buffer was nonempty)
 */
bool kb_buf_read(kb_buf_t *kb_buf, int *read_result);
/** @brief writes a keypress scancode into the keyboard buffer
 * 
 *  If the queue is full, false is returned. Otherwise, write the keypress event
 *  into the buffer and return true. The write_index is then incremented,
 *  wrapping back to 0 if necessary.
 *  
 *  @param kb_buf pointer to keyboard buffer to try to write keypress into
 *  @param keypress scancode to tryto write
 *  @return whether or not the write was successful (if the buffer was not full)
 */
bool kb_buf_write(kb_buf_t *kb_buf, int keypress);

#endif /* __KB_BUFFER_H_ */
