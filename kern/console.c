#include <p1kern.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <asm.h>

#include <simics.h>

#define ASCII_SPACE 0x20

int console_color = (FGND_GREEN | BGND_BLACK);
int console_row = 0;
int console_col = 0;
bool cursor_shown = true;

// what do i do about cursor color?

bool in_range(int row, int col)
{
    return row >= 0 && row < CONSOLE_HEIGHT && col >= 0 && col < CONSOLE_WIDTH;
}

void scroll()
{
    memmove((void*)CONSOLE_MEM_BASE,
            (void*)(CONSOLE_MEM_BASE + 2 * CONSOLE_WIDTH),
            (2 * (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH));
    
    char *last_row = (char*)(CONSOLE_MEM_BASE +
                             (2 * (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH));
    char *limit = last_row + (2 * CONSOLE_WIDTH);
    while (last_row < limit) {
        last_row[0] = ' ';
        last_row += 2;
    }
}

void write_char( char ch )
{
    char *write_addr;
    if (ch == '\r') {
        console_col = 0;
    }
    else if (ch == '\n') {
        console_col = 0;
        if (console_row == CONSOLE_HEIGHT - 1) {
            scroll();
        }
        else {
            console_row++;
        }
    }
    else if (ch == '\b') {
        if (console_col > 0) {
            console_col--;
            write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (console_row * CONSOLE_WIDTH + console_col));
            write_addr[0] = ASCII_SPACE;
            write_addr[1] = console_color;
        }
        else {
            if (console_row != 0) {
                console_row--;
                console_col = CONSOLE_WIDTH - 1;
                write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (console_row * CONSOLE_WIDTH + console_col));
                write_addr[0] = ASCII_SPACE;
                write_addr[1] = console_color;
            }
            // if console_row == 0, do nothing
        }
    }
    else {
        write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (console_row * CONSOLE_WIDTH + console_col));
        write_addr[0] = ch;
        write_addr[1] = console_color;
        if (console_col < CONSOLE_WIDTH - 1) {
            console_col++;
        }
        else {
            console_col = 0;
            if (console_row == CONSOLE_HEIGHT - 1) {
                scroll();
            }
            else {
                console_row++;
            }
        }
    }
}

int putbyte( char ch )
{
    write_char(ch);
    set_cursor(console_row, console_col);
    return ch;
}

void putbytes( const char *s, int len )
{
    if (len <= 0 || s == NULL) {
        return;
    }

    int i;
    for (i = 0; i < len; i++) {
        if (s[i] == '\0') {
            break;
        }
        write_char(s[i]);
    }

    set_cursor(console_row, console_col);
}

int set_term_color( int color )
{
    // considering blink in range
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
    console_row = row;
    console_col = col;

    if (cursor_shown) {
        show_cursor();
    }

    return 0;
}

void get_cursor( int *row, int *col )
{
    *row = console_row;
    *col = console_col;
}

void hide_cursor(void)
{
    cursor_shown = false;

    uint16_t addr = (uint16_t)(CONSOLE_HEIGHT * CONSOLE_WIDTH);
    uint8_t top_half = (addr >> 8) & 0xFF;
    uint8_t bottom_half = addr & 0xFF;

    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, top_half);
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, bottom_half);
}

void show_cursor(void)
{
    cursor_shown = true;

    uint16_t addr = console_row * CONSOLE_WIDTH + console_col;
    uint8_t top_half = (addr >> 8) & 0xFF;
    uint8_t bottom_half = addr & 0xFF;

    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, top_half);
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, bottom_half);
}

void clear_console(void)
{
    // we want to skip the second byte since that deals with color
    char *curr = (char*)CONSOLE_MEM_BASE;
    char *end = (char*)(CONSOLE_MEM_BASE + (2 * CONSOLE_HEIGHT * CONSOLE_WIDTH));
    while (curr < end) {
        curr[0] = ASCII_SPACE;
        curr += 2;
    }
    console_row = 0;
    console_col = 0;
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
    // the ascii range of printable chars as defined in keyhelp.h
    // if (!(ch >= 0x20 && ch <= 0x7F)) {
    //     return;
    // }
    char *write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (row * CONSOLE_WIDTH + col));
    write_addr[0] = ch;
    write_addr[1] = color;
}

char get_char( int row, int col )
{
    char *read_addr = (char*)(CONSOLE_MEM_BASE + 2 * (row * CONSOLE_WIDTH + col));
    return *read_addr;
}
