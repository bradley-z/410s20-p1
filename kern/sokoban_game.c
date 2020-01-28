#include <sokoban_game.h>
#include <stdint.h>
#include <p1kern.h>
#include <video_defines.h>
#include <stdio.h>

const char *ascii_sokoban = "\
   _____       _         _                 \
  / ____|     | |       | |                \
 | (___   ___ | | _____ | |__   __ _ _ __  \
  \\___ \\ / _ \\| |/ / _ \\| '_ \\ / _` | '_ \\ \
  ____) | (_) |   < (_) | |_) | (_| | | | |\
 |_____/ \\___/|_|\\_\\___/|_.__/ \\__,_|_| |_|";
const char *name = "Bradley Zhou (bradleyz)";
const char *intro_screen_message = "Press 'i' for instructions or 'enter' to start";
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

game_t game;

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

void display_introduction()
{
    draw_image(2, 18, 6, 43, FGND_BBLUE | BGND_BLACK, ascii_sokoban);
    draw_image(9, 28, 1, 23, FGND_YLLW | BGND_BLACK, name);
    draw_image(11, 17, 1, 46, FGND_WHITE | BGND_BLACK, intro_screen_message);
    draw_image(15, 9, 7, 13, FGND_BRWN | BGND_BLACK, ascii_left_crate);
    draw_image(15, 58, 7, 12, FGND_BRWN | BGND_BLACK, ascii_right_crate);

    set_cursor(15, 34);
    set_term_color(FGND_WHITE | BGND_BLACK);
    printf("Highscores:\n");
    set_cursor(16, 30);
    printf("1 - Moves: %u\n", game.hiscores[0].num_moves);
    set_cursor(17, 30);
    printf("    Time: %u\n", game.hiscores[0].time_seconds);
    set_cursor(18, 30);
    printf("2 - Moves: %u\n", game.hiscores[1].num_moves);
    set_cursor(19, 30);
    printf("    Time: %u\n", game.hiscores[1].time_seconds);
    set_cursor(20, 30);
    printf("3 - Moves: %u\n", game.hiscores[2].num_moves);
    set_cursor(21, 30);
    printf("    Time: %u\n", game.hiscores[2].time_seconds);
}

void sokoban_initialize_and_run()
{    
    score_t default_score = { UINT32_MAX, UINT32_MAX };
    game.hiscores[0] = default_score;
    game.hiscores[1] = default_score;
    game.hiscores[2] = default_score;

    display_introduction();
}