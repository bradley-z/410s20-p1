#include <p1kern.h>
#include <kb_buffer.h>
#include <keyhelp.h>

kb_buf_t kb_buffer;

int readchar(void)
{
    int curr_scancode;
    kh_type aug_char;
    while (kb_buf_read(&kb_buffer, &curr_scancode)) {
        aug_char = process_scancode(curr_scancode);
        if (KH_HASDATA(aug_char) && KH_ISMAKE(aug_char)) {
            return KH_GETCHAR(aug_char);
        }
    }
    return -1;
}
