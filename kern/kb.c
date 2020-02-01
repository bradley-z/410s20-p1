/** @file kb.c
 *  @brief keyboard driver implementation
 *
 *  Keyboard driver consists of only one function: readchar(). This function
 *  will continually poll a queue of keypresses until it determines a valid char
 *  has been pressed down, at which point, it returns that char. If there are no
 *  keypresses, the function returns -1 immediately.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug No known bugs.
 */
#include <p1kern.h>     /* declaration for readchar() */
#include <kb_buffer.h>  /* kb_buf_t, kb_buf_read() */
#include <keyhelp.h>    /* kh_type, KH_HASDATA(), KH_ISMAKE(), KH_GETCHAR() */

/**
 * global keyboard buffer that we poll for new keypresses
 */
kb_buf_t kb_buffer;

int readchar(void)
{
    int curr_scancode;
    kh_type aug_char;
    /* while the buffer is not empty */
    while (kb_buf_read(&kb_buffer, &curr_scancode)) {
        aug_char = process_scancode(curr_scancode);
        if (KH_HASDATA(aug_char) && KH_ISMAKE(aug_char)) {
            return KH_GETCHAR(aug_char);
        }
    }
    return -1;
}
