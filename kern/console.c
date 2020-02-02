/** @file console.c
 *  @brief console driver implementation
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug '\b' isn't handled perfectly. If we are typing some text onto a line
 *       and partway through, we press '\n' to move onto the next line, the
 *       console doesn't "remember" where the last position was on the previous
 *       line, so if we press backspace immediately after that, the cursor will
 *       move to the last character in the previous line (which is most likely
 *       an empty space) instead of the position right before the '\n'.
 */
#include <p1kern.h>
#include <stddef.h>     /* NULL */
#include <stdbool.h>    /* bool */
#include <stdint.h>     /* uint8_t, uint16_t */
#include <string.h>     /* memmove */
#include <asm.h>        /* outb */

/* ASCII code for space character */
#define ASCII_SPACE 0x20

/* global variables keeping track of the state of the console/cursor */
int console_color = (FGND_WHITE | BGND_BLACK);
int cursor_row = 0;
int cursor_col = 0;
bool cursor_shown = true;

/** @brief checks if row/col are in range of the console
 *
 *  Compares row and col with CONSOLE_HEIGHT and CONSOLE_WIDTH.
 *
 *  @return if the row and col are in range of the console
 */
static bool in_range(int row, int col)
{
    return row >= 0 && row < CONSOLE_HEIGHT && col >= 0 && col < CONSOLE_WIDTH;
}

/** @brief scrolls the console up one line
 *
 *  Moves the memory from rows [1, CONSOLE_HEIGHT) to [0, CONSOLE_HEIGHT - 1)
 *  and flushes the botttom line of the console with all spaces, clearing it.
 *
 *  @return Void.
 */
static void scroll()
{
    memmove((void*)CONSOLE_MEM_BASE,
            /* start from row 1 */
            (void*)(CONSOLE_MEM_BASE + 2 * CONSOLE_WIDTH),
            /* amount of bytes from CONSOLE_HEIGHT - 1 lines */
            (2 * (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH));

    char *last_row = (char*)(CONSOLE_MEM_BASE +
                             (2 * (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH));
    char *limit = last_row + (2 * CONSOLE_WIDTH);
    while (last_row < limit) {
        last_row[0] = ASCII_SPACE;
        /* write every two bytes beacuse we don't want to overwrite color */
        last_row += 2;
    }
}

/** @brief all logic for putbyte() just without setting the cursor immediately
 *
 *  The motivation behind this function is because the most obvious approach for
 *  putbytes() is to just call putbyte() a number of times. However, my
 *  implementation for putbyte() sets the cursor immediately. Therefore, the
 *  obvious putbytes() implementation would call set_cursor() an unnecessary
 *  amount of times. Pulling out the logic into write_char() means we can just
 *  call set_cursor() once in putbytes().
 *
 *  '\b' at the start of a line will start deleting characters from the previous
 *  line. If we are at the start of of the line at the first row and we get a
 *  '\b', then nothing happens.
 *
 *  @param ch character to write to console
 */
static void write_char( char ch )
{
    char *write_addr;
    if (ch == '\r') {
        cursor_col = 0;
    }
    else if (ch == '\n') {
        cursor_col = 0;
        /* scroll if we called '\n' when we're on the last line */
        if (cursor_row == CONSOLE_HEIGHT - 1) {
            scroll();
        }
        else {
            cursor_row++;
        }
    }
    else if (ch == '\b') {
        /* "normal" case where we can just write a space over prev character */
        if (cursor_col > 0) {
            cursor_col--;
        }
        else {
            if (cursor_row == 0) {
                return;
            }
            cursor_row--;
            cursor_col = CONSOLE_WIDTH - 1;
        }
        write_addr = (char*)(CONSOLE_MEM_BASE +
                     2 * (cursor_row * CONSOLE_WIDTH + cursor_col));
        write_addr[0] = ASCII_SPACE;
        write_addr[1] = console_color;
    }
    else {
        write_addr = (char*)(CONSOLE_MEM_BASE +
                     2 * (cursor_row * CONSOLE_WIDTH + cursor_col));
        write_addr[0] = ch;
        write_addr[1] = console_color;
        /* increment the cursor position, next row/scrolling if necessary */
        if (cursor_col < CONSOLE_WIDTH - 1) {
            cursor_col++;
        }
        else {
            cursor_col = 0;
            if (cursor_row == CONSOLE_HEIGHT - 1) {
                scroll();
            }
            else {
                cursor_row++;
            }
        }
    }
}

int putbyte( char ch )
{
    write_char(ch);
    set_cursor(cursor_row, cursor_col);
    return ch;
}

void putbytes( const char *s, int len )
{
    if (len <= 0 || s == NULL) {
        return;
    }

    int i;
    for (i = 0; i < len; i++) {
        /* break early if we reach null terminator */
        if (s[i] == '\0') {
            break;
        }
        write_char(s[i]);
    }

    set_cursor(cursor_row, cursor_col);
}

int set_term_color( int color )
{
    /* considering blink in color range */
    if ((unsigned int)color > 0xFF) {
        return -1;
    }
    console_color = color;
    return 0;
}

void get_term_color( int *color )
{
    *color = console_color;
}

int set_cursor( int row, int col )
{
    if (!in_range(row, col)) {
        return -1;
    }
    cursor_row = row;
    cursor_col = col;

    if (cursor_shown) {
        show_cursor();
    }

    return 0;
}

void get_cursor( int *row, int *col )
{
    *row = cursor_row;
    *col = cursor_col;
}

void hide_cursor(void)
{
    cursor_shown = false;

    uint16_t addr = (uint16_t)(CONSOLE_HEIGHT * CONSOLE_WIDTH);
    uint8_t top_half = (addr >> 8) & 0xFF;
    uint8_t bottom_half = addr & 0xFF;

    /* writes cursor to just out of range of the console */
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, top_half);
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, bottom_half);
}

void show_cursor(void)
{
    cursor_shown = true;

    uint16_t addr = cursor_row * CONSOLE_WIDTH + cursor_col;
    uint8_t top_half = (addr >> 8) & 0xFF;
    uint8_t bottom_half = addr & 0xFF;

    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, top_half);
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, bottom_half);
}

void clear_console(void)
{
    register char *curr = (char*)CONSOLE_MEM_BASE;
    char *end = (char*)(CONSOLE_MEM_BASE +
                        (2 * CONSOLE_HEIGHT * CONSOLE_WIDTH));
    /* only write every two bytes because we don't want to overwrite color */
    while (curr < end) {
        curr[0] = ASCII_SPACE;
        curr += 2;
    }
    cursor_row = 0;
    cursor_col = 0;
    if (cursor_shown) {
        show_cursor();
    }
}

void draw_char( int row, int col, int ch, int color )
{
    if (!in_range(row, col)) {
        return;
    }
    if ((unsigned int)color > 0xFF) {
        return;
    }
    char *write_addr = (char*)(CONSOLE_MEM_BASE +
                               2 * (row * CONSOLE_WIDTH + col));
    write_addr[0] = ch;
    write_addr[1] = color;
}

char get_char( int row, int col )
{
    char *read_addr = (char*)(CONSOLE_MEM_BASE +
                              2 * (row * CONSOLE_WIDTH + col));
    return *read_addr;
}
