#include <p1kern.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <asm.h>

#include <simics.h>

#define ASCII_SPACE 0x20;

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
    int i, j;
    for (i = 0; i < CONSOLE_HEIGHT - 1; i++) {
        for (j = 0; j < CONSOLE_WIDTH; j++) {
            char *old_addr = (char*)(CONSOLE_MEM_BASE + 2 * (i * CONSOLE_WIDTH + j));
            char *new_addr = (char*)(CONSOLE_MEM_BASE + 2 * ((i + 1) * CONSOLE_WIDTH + j));
            old_addr[0] = new_addr[0];
            old_addr[1] = new_addr[1];
        }
    }
}

int putbyte( char ch )
{
    char *write_addr;
    if (ch == '\r') {
        console_col = 0;
    }
    else if (ch == '\n') {
        console_col = 0;
        if (console_row == CONSOLE_WIDTH - 1) {
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
        } else {
            if (console_row == 0) {
                set_cursor(console_row, console_col);
                return ch;
            } else {
                console_row--;
                console_col = CONSOLE_WIDTH - 1;
                write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (console_row * CONSOLE_WIDTH + console_col));
                write_addr[0] = ASCII_SPACE;
                write_addr[1] = console_color;
            }
        }
    }
    else {
        if (console_col < CONSOLE_WIDTH - 1) {
            write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (console_row * CONSOLE_WIDTH + console_col));
            write_addr[0] = ch;
            write_addr[1] = console_color;
            console_col++;
        }
        else {
            console_col = 0;
            if (console_row == CONSOLE_HEIGHT - 1) {
                scroll();
            }
            write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (console_row * CONSOLE_WIDTH + console_col));
            write_addr[0] = ch;
            write_addr[1] = console_color;
        }
    }
    set_cursor(console_row, console_col);
    return ch;
}

void
putbytes( const char *s, int len )
{
    if (len <= 0 || s == NULL) {
        return;
    }

    int i;
    for (i = 0; i < len; i++) {
        putbyte(s[i]);
    }
}

int
set_term_color( int color )
{
    // is blink in range?
    if (color > 0x7F) {
        return -1;
    }
    console_color = color;
    return 0;
}

void
get_term_color( int *color )
{
    *color = console_color;
}

int
set_cursor( int row, int col )
{
    if (!in_range(row, col)) {
        return -1;
    }
    console_row = row;
    console_col = col;

    if (cursor_shown) {
        uint16_t addr = row * CONSOLE_WIDTH + col;
        uint8_t top_half = (addr >> 8) & 0xFF;
        uint8_t bottom_half = addr & 0xFF;

        outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
        outb(CRTC_DATA_REG, top_half);
        outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
        outb(CRTC_DATA_REG, bottom_half);
    }

    return 0;
}

void
get_cursor( int *row, int *col )
{
    *row = console_row;
    *col = console_col;
}

void
hide_cursor(void)
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

void
show_cursor(void)
{
    cursor_shown = true;
    set_cursor(console_row, console_col);
}

void
clear_console(void)
{
    // no we want to skip the second byte since that deals with color
    char *curr = (char*)CONSOLE_MEM_BASE;
    char *end = (char*)(CONSOLE_MEM_BASE + CONSOLE_HEIGHT * CONSOLE_WIDTH);
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

// what do i do about \r or \b or \n and where does cursor go after?
void
draw_char( int row, int col, int ch, int color )
{
    if (!in_range(row, col)) {
        return;
    }
    if (color > 0x7F) {
        return;
    }
    char *write_addr = (char*)(CONSOLE_MEM_BASE + 2 * (row * CONSOLE_WIDTH + col));
    write_addr[0] = ch;
    write_addr[1] = color;
}

char
get_char( int row, int col )
{
    char *read_addr = (char*)(CONSOLE_MEM_BASE + 2 * (row * CONSOLE_WIDTH + col));
    return *read_addr;
}
