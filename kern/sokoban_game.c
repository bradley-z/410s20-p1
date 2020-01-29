#include <sokoban_game.h>
#include <stdint.h>
#include <p1kern.h>
#include <video_defines.h>
#include <stdio.h>
#include <string.h>
#include <keyset.h>
#include <sokoban.h>

#define CONSOLE_SIZE (2 * CONSOLE_HEIGHT * CONSOLE_WIDTH)
char saved_screen[CONSOLE_SIZE];

const char *ascii_sokoban = "\
   _____       _         _                 \
  / ____|     | |       | |                \
 | (___   ___ | | _____ | |__   __ _ _ __  \
  \\___ \\ / _ \\| |/ / _ \\| '_ \\ / _` | '_ \\ \
  ____) | (_) |   < (_) | |_) | (_| | | | |\
 |_____/ \\___/|_|\\_\\___/|_.__/ \\__,_|_| |_|";
const char *name = "Bradley Zhou (bradleyz)";
const char *intro_screen_message = "Press 'i' for instructions or 'enter' to start";
const char *game_screen_message = "Press 'i' for instructions, 'p' to pause, 'r' to restart, or 'q' to quit";
const char *ascii_left_crate = "\
    .+------+\
  .' |    .'|\
 +---+--+'  |\
 |   |  |   |\
 |  ,+--+---+\
 |.'    | .' \
 +------+'   ";
const char *ascii_right_crate = "\
+------+.   \
|`.    | `. \
|  `+--+---+\
|   |  |   |\
+---+--+   |\
 `. |   `. |\
   `+------+";

game_t current_game;
keyset_t keyset;
sokoban_t sokoban;

void sokoban_tickback(unsigned int numTicks)
{
    if (sokoban.state != LEVEL_RUNNING) {
        return;
    }
    if (current_game.game_state != RUNNING) {
        return;
    }
}

int16_t draw_sokoban_level(sokolevel_t *level)
{
    clear_console();

    int curr_draw_row = (CONSOLE_HEIGHT - level->height) / 2;
    int curr_draw_col = (CONSOLE_WIDTH - level->width) / 2;
    int start_draw_col = curr_draw_col;
    int end_draw_col = curr_draw_col + level->width - 1;

    int total_pixels = level->height * level->width;
    int pixel_count;

    int8_t level_start_row = 0;
    int8_t level_start_col = 0;

    for (pixel_count = 0; pixel_count < total_pixels; pixel_count++) {
        char ch = level->map[pixel_count];
        char color;

        switch (ch) {
            case (SOK_WALL):
                color = FGND_DGRAY | BGND_BLACK;
                break;
            case (SOK_PUSH):
                level_start_row = curr_draw_row;
                level_start_col = curr_draw_col;
                color = FGND_BCYAN | BGND_BLACK;
                break;
            case (SOK_ROCK):
                color = FGND_BRWN | BGND_BLACK;
                break;
            case (SOK_GOAL):
                color = FGND_YLLW | BGND_BLACK;
                break;
            default:
                if (curr_draw_col == end_draw_col) {
                    curr_draw_col = start_draw_col;
                    curr_draw_row++;
                }
                else {
                    curr_draw_col++;
                }
                continue;
        }

        draw_char(curr_draw_row, curr_draw_col, ch, color);
        if (curr_draw_col == end_draw_col) {
            curr_draw_col = start_draw_col;
            curr_draw_row++;
        }
        else {
            curr_draw_col++;
        }
    }

    int message_row = CONSOLE_HEIGHT - ((CONSOLE_HEIGHT - curr_draw_row) / 2) - 1;
    set_cursor(message_row, 4);
    printf(game_screen_message);
    print_current_game_time();

    return (int16_t)level_start_row << 8 | (int16_t)level_start_col;
}

void print_current_game_time()
{
    set_cursor(2, 4);
    printf("Time: %d", current_game.num_ticks / 100);
}

void handle_input(char ch)
{
    switch (ch) {
        case 'i':
            if (sokoban.state == INTRODUCTION) {
                display_instructions();
            }
            else if (sokoban.state == INSTRUCTIONS) {
                if (sokoban.previous_state == INTRODUCTION) {
                    display_introduction();
                }
                else if (sokoban.previous_state == LEVEL_RUNNING) {
                    memcpy((void*)CONSOLE_MEM_BASE, (void*)saved_screen, CONSOLE_SIZE);
                    sokoban.previous_state = sokoban.state;
                    sokoban.state = LEVEL_RUNNING;
                    current_game.game_state = RUNNING;
                }
            }
            else if (sokoban.state == LEVEL_RUNNING) {
                current_game.game_state = PAUSED;
                memcpy((void*)saved_screen, (void*)CONSOLE_MEM_BASE, CONSOLE_SIZE);
                display_instructions();
            }
            break;
        case '\n':
            if (sokoban.state == INTRODUCTION) {
               start_game(); 
            }
            break;
        case 'p':
            if (sokoban.state == LEVEL_RUNNING) {
                if (current_game.game_state == RUNNING) {
                    memcpy((void*)saved_screen, (void*)CONSOLE_MEM_BASE, CONSOLE_SIZE);
                    pause_game();
                }
                else if (current_game.game_state == PAUSED) {
                    memcpy((void*)CONSOLE_MEM_BASE, (void*)saved_screen, CONSOLE_SIZE);
                    current_game.game_state = RUNNING;
                }
            }
            break;
        default:
            break;
    }
}

char poll_for_input()
{
    int ch;
    do {
        ch = readchar();
    } while (ch == -1 && !keyset_contains(&keyset, (char)ch));
    return ch;
}

void draw_image(int start_row, int start_col,
                int height, int width,
                int color, const char *image)
{
    int i, j;
    int end_row = start_row + height;
    int end_col = start_col + width;
    int pixel_count = 0;
    for (i = start_row; i < end_row; i++) {
        for (j = start_col; j < end_col; j++) {
            draw_char(i, j, image[pixel_count], color);
            pixel_count++;
        }
    }
}

void pause_game()
{
    current_game.game_state = PAUSED;
    clear_console();

    set_cursor(0, 0);
    printf("Paused!\n\nPress 'p' to unpause");
}

void start_game()
{
    sokoban.previous_state = sokoban.state;
    sokoban.state = LEVEL_RUNNING;

    int16_t position = draw_sokoban_level(soko_levels[0]);
    int row = (position >> 8) & 0xFF;
    int col = position & 0xFF;

    current_game.level = soko_levels[0];
    current_game.level_number = 0;
    current_game.cur_row = row;
    current_game.cur_col = col;
    current_game.num_ticks = 0;
    current_game.total_moves = 0;
    current_game.level_moves = 0;
    current_game.game_state = RUNNING;
}

void display_instructions()
{
    sokoban.previous_state = sokoban.state;
    sokoban.state = INSTRUCTIONS;
    clear_console();
    set_cursor(0, 0);
    printf("Instructions!\n\nPress 'i' to return");
}

void display_introduction()
{
    sokoban.previous_state = sokoban.state;
    sokoban.state = INTRODUCTION;
    clear_console();
    draw_image(2, 18, 6, 43, FGND_BBLUE | BGND_BLACK, ascii_sokoban);
    draw_image(9, 28, 1, 23, FGND_YLLW | BGND_BLACK, name);
    draw_image(11, 17, 1, 46, FGND_WHITE | BGND_BLACK, intro_screen_message);
    draw_image(15, 9, 7, 13, FGND_BRWN | BGND_BLACK, ascii_left_crate);
    draw_image(15, 58, 7, 12, FGND_BRWN | BGND_BLACK, ascii_right_crate);

    set_cursor(15, 34);
    set_term_color(FGND_WHITE | BGND_BLACK);
    printf("Highscores:\n");
    set_cursor(16, 30);
    printf("1 - Moves: %u\n", sokoban.hiscores[0].num_moves);
    set_cursor(17, 30);
    printf("    Time: %u\n", sokoban.hiscores[0].time_seconds);
    set_cursor(18, 30);
    printf("2 - Moves: %u\n", sokoban.hiscores[1].num_moves);
    set_cursor(19, 30);
    printf("    Time: %u\n", sokoban.hiscores[1].time_seconds);
    set_cursor(20, 30);
    printf("3 - Moves: %u\n", sokoban.hiscores[2].num_moves);
    set_cursor(21, 30);
    printf("    Time: %u\n", sokoban.hiscores[2].time_seconds);
}

void sokoban_initialize_and_run()
{    
    keyset_initialize(&keyset);
    keyset_insert(&keyset, 'i');
    keyset_insert(&keyset, '\n');
    keyset_insert(&keyset, 'p');
    keyset_insert(&keyset, 'w');
    keyset_insert(&keyset, 'a');
    keyset_insert(&keyset, 's');
    keyset_insert(&keyset, 'd');

    score_t default_score = { UINT32_MAX, UINT32_MAX };
    sokoban.hiscores[0] = default_score;
    sokoban.hiscores[1] = default_score;
    sokoban.hiscores[2] = default_score;
    sokoban.state = INTRODUCTION;
    sokoban.previous_state = INTRODUCTION;

    display_introduction();

    char c;
    while (1) {
        c = poll_for_input();
        handle_input(c);
    }
}