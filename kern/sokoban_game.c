#include <sokoban_game.h>
#include <p1kern.h>
#include <sokoban.h>
#include <stdbool.h>
#include <stdint.h>         /* UINT32_MAX */
#include <video_defines.h>  /* console size, color constants */
#include <stdio.h>          /* printf */
#include <string.h>         /* memcpy */

#define ASCII_SPACE         0x20

#define MY_SOK_WALL         ((char)0xB0)
#define MY_SOK_PLAYER       ('@')
#define MY_SOK_BOX          ('o')
#define MY_SOK_GOAL         ('x')
#define MY_SOK_BOX_ON_GOAL  ('O')

#define DEFAULT_COLOR       (FGND_WHITE | BGND_BLACK)
#define PLAYER_COLOR        (FGND_BCYAN | BGND_BLACK)
#define BOX_COLOR           (FGND_BRWN | BGND_BLACK)
#define GOAL_COLOR          (FGND_YLLW | BGND_BLACK)
#define BOX_ON_GOAL_COLOR   (FGND_GREEN | BGND_BLACK)

#define ALIGNMENT_TWENTYTH  5
#define ALIGNMENT_TWELFTH   8
#define ALIGNMENT_TENTH     10
#define ALIGNMENT_EIGHT     12
#define ALIGNMENT_SIXTH     17
#define ALIGNMENT_QUARTER   25
#define ALIGNMENT_THIRD     33
#define ALIGNMENT_3EIGHTS   38
#define ALIGNMENT_HALF      50
#define MAX_PERCENT         100

#define ASCII_SOKO_HEIGHT   6
#define ASCII_SOKO_WIDTH    43
#define ASCII_LBOX_HEIGHT   7
#define ASCII_LBOX_WIDTH    13
#define ASCII_RBOX_HEIGHT   7
#define ASCII_RBOX_WIDTH    12

#define STRING_HEIGHT       1
#define ELEMENT_ROW_SPACING 1

#define CONSOLE_SIZE        (2 * CONSOLE_HEIGHT * CONSOLE_WIDTH)

static inline int align_row(alignment_t alignment, int height, int percentage);
static inline int align_col(alignment_t alignment, int width, int percentage);

static void draw_image(const char *image, int start_row, int start_col,
                       int height, int width, int color);
static bool draw_sokoban_level(sokolevel_t *level, int *total_boxes,
                               int *start_row, int *start_col);
static void put_time_at_loc(int ticks, int row, int col);
static void print_current_game_moves(void);
static void print_current_game_time(void);
static void putstring(const char *str, int row, int col, int color);

static bool valid_next_square(dir_t dir, int row, int col,
                              int *new_row, int *new_col);
static void try_move(dir_t dir);
static void handle_input(char ch);
static void level_up(void);
static void complete_game(void);
static void complete_level(void);
static void quit_game(void);
static void pause_game(void);
static void restart_current_level(void);
static void start_sokoban_level(int level_number);
static void start_game(void);
static void display_instructions(void);
static void display_introduction(void);

const char *ascii_sokoban = "\
   _____       _         _                 \
  / ____|     | |       | |                \
 | (___   ___ | | _____ | |__   __ _ _ __  \
  \\___ \\ / _ \\| |/ / _ \\| '_ \\ / _` | '_ \\ \
  ____) | (_) |   < (_) | |_) | (_| | | | |\
 |_____/ \\___/|_|\\_\\___/|_.__/ \\__,_|_| |_|";
const char *ascii_left_box = "\
    .+------+\
  .' |    .'|\
 +---+--+'  |\
 |   |  |   |\
 |  ,+--+---+\
 |.'    | .' \
 +------+'   ";
const char *ascii_right_box = "\
+------+.   \
|`.    | `. \
|  `+--+---+\
|   |  |   |\
+---+--+   |\
 `. |   `. |\
   `+------+";

const char *name = "Bradley Zhou (bradleyz)";
const char *intro_screen_message =
                            "Press 'i' for instructions or 'enter' to start";
const char *game_screen_message =
    "Press 'i' for instructions, 'p' to pause, 'r' to restart, or 'q' to quit";
const char *summary_screen_message = "Press any key to continue";
const char *game_complete_message =
                            "Press any key to return to introduction screen";
const char *pause_screen_message = "Press 'p' to unpause";
const char *end_level_messages[] = {
    "Phase 1 defused. How about the next one?",
    "That's number 2. Keep going!",
    "Halfway there!",
    "So you got that one. Try this one.",
    "Good work! On to the next...",
    "Congratulations! You've defused the bomb! Wait... wrong class...",
    0,
};

const char *instructions[] = {
    "0. You are represented by '@', boxes by 'o', and target locations by 'x'",
    "1. Use WASD or HJKL to either move onto an empty square or push a box",
    "2. You can push boxes onto empty squares and target locations",
    "3. Boxes cannot be pulled, or pushed into other boxes or walls",
    "4. There is an equal number of boxes and target locations",
    "5. Push each box into its own target location to complete the level",
    "6. Complete all six levels to complete the game",
    0,
};

char saved_screen[CONSOLE_SIZE];
char timer_print_buf[CONSOLE_WIDTH];

game_t current_game;
sokoban_t sokoban;

static inline int align_row(alignment_t alignment, int height, int percentage)
{
    if (height < 0 || height >= CONSOLE_HEIGHT ||
        percentage < 0 || percentage > MAX_PERCENT) {
        return -1;
    }

    switch (alignment) {
        case TOP_SIDE:
            return (CONSOLE_HEIGHT * percentage / MAX_PERCENT);
        case CENTER:
            return ((CONSOLE_HEIGHT - height) * percentage) / MAX_PERCENT;
        // this can return negative
        case BOTTOM_SIDE:
            return ((CONSOLE_HEIGHT - 1) -
                   (CONSOLE_HEIGHT * percentage) / MAX_PERCENT -
                   height);
        default:
            return -1;
    }
}

static inline int align_col(alignment_t alignment, int width, int percentage)
{
    if (width < 0 || width >= CONSOLE_WIDTH ||
        percentage < 0 || percentage > MAX_PERCENT) {
        return -1;
    }

    switch (alignment) {
        case LEFT_SIDE:
            return (CONSOLE_WIDTH * percentage / MAX_PERCENT);
        case CENTER:
            return ((CONSOLE_WIDTH - width) * percentage) / MAX_PERCENT;
        case RIGHT_SIDE:
            // this can return negative
            return ((CONSOLE_WIDTH - 1) -
                   (CONSOLE_WIDTH * percentage / MAX_PERCENT) -
                   width);
        default:
            return -1;
    }
}

void sokoban_tickback()
{
    if (sokoban.state != LEVEL_RUNNING) {
        return;
    }
    if (current_game.game_state != RUNNING) {
        return;
    }

    if (++current_game.level_ticks % 10 == 0) {
        print_current_game_time();
    } 
}

static void draw_image(const char *image, int start_row, int start_col,
                       int height, int width, int color)
{
    if (image == NULL) {
        return;
    }

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

static bool draw_sokoban_level(sokolevel_t *level, int *total_boxes,
                               int *start_row, int *start_col)
{
    if (total_boxes == NULL || start_row == NULL || start_col == NULL) {
        return false;
    }

    clear_console();

    set_cursor(1, 4);
    printf("Level: %d", current_game.level_number);

    int curr_row = align_row(CENTER, level->height, ALIGNMENT_HALF);
    int curr_col = align_col(CENTER, level->width, ALIGNMENT_HALF);
    int first_col = curr_col;
    int end_col = curr_col + level->width - 1;

    int total_pixels = level->height * level->width;
    int pixel_count;

    bool found_start = false;
    int num_boxes = 0;

    for (pixel_count = 0; pixel_count < total_pixels; pixel_count++) {
        char ch = level->map[pixel_count];
        char color;

        switch (ch) {
            case (SOK_WALL):
                color = FGND_DGRAY | BGND_BLACK;
                ch = MY_SOK_WALL;
                break;
            case (SOK_PUSH):
                *start_row = curr_row;
                *start_col = curr_col;
                color = FGND_BCYAN | BGND_BLACK;
                ch = MY_SOK_PLAYER;
                found_start = true;
                break;
            case (SOK_ROCK):
                num_boxes++;
                color = FGND_BRWN | BGND_BLACK;
                ch = MY_SOK_BOX;
                break;
            case (SOK_GOAL):
                color = FGND_YLLW | BGND_BLACK;
                ch = MY_SOK_GOAL;
                break;
            default:
                if (curr_col == end_col) {
                    curr_col = first_col;
                    curr_row++;
                }
                else {
                    curr_col++;
                }
                continue;
        }

        draw_char(curr_row, curr_col, ch, color);
        if (curr_col == end_col) {
            curr_col = first_col;
            curr_row++;
        }
        else {
            curr_col++;
        }
    }
    *total_boxes = num_boxes;

    if (num_boxes == 0 || !found_start) {
        return false;
    }

    int message_row = CONSOLE_HEIGHT - ((CONSOLE_HEIGHT - curr_row) / 2) - 1;
    putstring(game_screen_message, message_row, 4, DEFAULT_COLOR);
    putstring("Moves: ", 3, 4, DEFAULT_COLOR);
    putstring("Time: ", 4, 4, DEFAULT_COLOR);
    print_current_game_moves();
    print_current_game_time();

    return true;
}

static void put_time_at_loc(int ticks, int row, int col)
{
    int len = snprintf(timer_print_buf, CONSOLE_WIDTH, "%d", ticks / 10);
    timer_print_buf[len + 1] = '\0';
    timer_print_buf[len] = timer_print_buf[len - 1];
    timer_print_buf[len - 1] = '.';
    putstring(timer_print_buf, row, col, DEFAULT_COLOR);
}

static void print_current_game_moves()
{
    set_cursor(3, 11);
    printf("%d", current_game.level_moves);
}

static void print_current_game_time()
{
    put_time_at_loc(current_game.level_ticks, 4, 10);
}

static void putstring(const char *str, int row, int col, int color)
{
    if (str == NULL) {
        return;
    }

    int old_color;
    get_term_color(&old_color);

    set_term_color(color);
    set_cursor(row, col);

    while (*str != '\0') {
        putbyte(*str);
        str++;
    }

    set_term_color(old_color);
}

static bool valid_next_square(dir_t dir, int row, int col,
                              int *new_row, int *new_col)
{
    if (new_row == NULL || new_col == NULL) {
        return false;
    }

    switch (dir) {
        case UP:
            if (row - 1 < 0) {
                return false;
            }
            *new_row = row - 1;
            *new_col = col;
            break;
        case DOWN:
            if (row + 1 >= CONSOLE_HEIGHT) {
                return false;
            }
            *new_row = row + 1;
            *new_col = col;
            break;
        case LEFT:
            if (col - 1 < 0) {
                return false;
            }
            *new_row = row;
            *new_col = col - 1;
            break;
        case RIGHT:
            if (col + 1 >= CONSOLE_WIDTH - 1) {
                return false;
            }
            *new_row = row;
            *new_col = col + 1;
            break;
        default:
            return false;
    }

    return true;
}

static void try_move(dir_t dir)
{
    int row = current_game.curr_row;
    int col = current_game.curr_col;

    int new_row, new_col;
    if (!valid_next_square(dir, row, col, &new_row, &new_col)) {
        return;
    }

    char next_square = get_char(new_row, new_col);
    bool movable = (next_square == ASCII_SPACE) ||
                   (next_square == MY_SOK_GOAL);
    bool pushable = (next_square == MY_SOK_BOX) ||
                    (next_square == MY_SOK_BOX_ON_GOAL);

    if (movable) {
        if (current_game.on_goal) {
            draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
            if (next_square == ASCII_SPACE) {
                current_game.on_goal = false;
            }
        }
        else {
            draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
            if (next_square == MY_SOK_GOAL) {
                current_game.on_goal = true;
            }
        }

        draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
        current_game.level_moves++;
        current_game.curr_row = new_row;
        current_game.curr_col = new_col;
    }
    else if (pushable) {
        int next_next_square_row, next_next_square_col;
        if (!valid_next_square(dir, new_row, new_col,
                               &next_next_square_row, &next_next_square_col)) {
            return;
        }

        char next_next_square = get_char(next_next_square_row,
                                         next_next_square_col);
        bool next_next_movable = (next_next_square == ASCII_SPACE) ||
                                 (next_next_square == MY_SOK_GOAL);

        if (next_next_movable) {
            if (current_game.on_goal) {
                draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
                if (next_square == MY_SOK_BOX) {
                    current_game.on_goal = false;
                }
            }
            else {
                draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
                if (next_square == MY_SOK_BOX_ON_GOAL) {
                    current_game.on_goal = true;
                }
            }
            draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
            if (next_next_square == ASCII_SPACE) {
                draw_char(next_next_square_row, next_next_square_col,
                          MY_SOK_BOX, BOX_COLOR);
            }
            else if (next_next_square == MY_SOK_GOAL) {
                draw_char(next_next_square_row, next_next_square_col,
                          MY_SOK_BOX_ON_GOAL, BOX_ON_GOAL_COLOR);
            }
            current_game.level_moves++;
            current_game.curr_row = new_row;
            current_game.curr_col = new_col;

            if (next_square == MY_SOK_BOX && next_next_square == MY_SOK_GOAL) {
                current_game.boxes_left--;
            }
            if (next_square == MY_SOK_BOX_ON_GOAL &&
                next_next_square == ASCII_SPACE) {
                current_game.boxes_left++;
            }
        }
    }
    print_current_game_moves();

    if (current_game.boxes_left == 0) {
        complete_level();
    }
}

static void handle_input(char ch)
{
    sokoban_state_t state = sokoban.state;

    if (state == INTRODUCTION) {
        switch (ch) {
            case 'i':
                display_instructions();
                break;
            case '\n':
                start_game();
                break;
            default:
                return;
        }
    }
    else if (state == INSTRUCTIONS) {
        switch (ch) {
            case 'i':
                if (sokoban.previous_state == INTRODUCTION) {
                    display_introduction();
                }
                else if (sokoban.previous_state == LEVEL_RUNNING) {
                    memcpy((void*)CONSOLE_MEM_BASE,
                           (void*)saved_screen, CONSOLE_SIZE);
                    sokoban.state = LEVEL_RUNNING;
                    current_game.game_state = RUNNING;
                }
                break;
            default:
                return;
        }
    }
    else if (state == LEVEL_RUNNING) {
        game_state_t game_state = current_game.game_state;

        if (game_state == IN_LEVEL_SUMMARY) {
            level_up();
        }
        else if (game_state == PAUSED) {
            if (ch == 'p') {
                memcpy((void*)CONSOLE_MEM_BASE,
                       (void*)saved_screen, CONSOLE_SIZE);
                current_game.game_state = RUNNING;
            }
        }
        else if (game_state == RUNNING) {
            switch (ch) {
                case 'i':
                    memcpy((void*)saved_screen,
                           (void*)CONSOLE_MEM_BASE, CONSOLE_SIZE);
                    display_instructions();
                    break;
                case 'p':
                    memcpy((void*)saved_screen,
                           (void*)CONSOLE_MEM_BASE, CONSOLE_SIZE);
                    pause_game();
                    break;
                case 'q':
                    quit_game();
                    break;
                case 'r':
                    restart_current_level();
                    break;
                case 'w':
                case 'k':
                    try_move(UP);
                    break;
                case 's':
                case 'j':
                    try_move(DOWN);
                    break;
                case 'a':
                case 'h':
                    try_move(LEFT);
                    break;
                case 'd':
                case 'l':
                    try_move(RIGHT);
                    break;
                default:
                    break;
            }
        }
    }
}

static void level_up()
{
    if (current_game.level_number == soko_nlevels) {
        display_introduction();
    }
    else {
        current_game.level_number++;
        start_sokoban_level(current_game.level_number);
    }
}

static void complete_game()
{
    score_t score = { current_game.total_moves, current_game.total_ticks };

    int i;
    for (i = 0; i < 3; i++) {
        if (score.num_moves < sokoban.hiscores[i].num_moves ||
           (score.num_moves == sokoban.hiscores[i].num_moves &&
            score.num_ticks < sokoban.hiscores[i].num_ticks)) {
            int j;
            for (j = 2; j > i; j--) {
                sokoban.hiscores[j] = sokoban.hiscores[j - 1];
            }
            sokoban.hiscores[i] = score;
            break;
        }
    }

    clear_console();
    const char *msg = end_level_messages[current_game.level_number - 1];
    int msg_len = strlen(msg);

    putstring(msg, 11, (CONSOLE_WIDTH - msg_len) / 2, FGND_YLLW | BGND_BLACK);
    putstring(game_complete_message, 19, 17, FGND_MAG | BGND_BLACK);

    set_term_color(DEFAULT_COLOR);
    set_cursor(13, 32);
    printf("Total moves: %d", current_game.total_moves);
    putstring("Total time: ", 14, 32, DEFAULT_COLOR);
    put_time_at_loc(current_game.total_ticks, 14, 44);
}

static void complete_level()
{
    current_game.game_state = IN_LEVEL_SUMMARY;

    current_game.total_ticks += current_game.level_ticks;
    current_game.total_moves += current_game.level_moves;
    if (current_game.level_number == soko_nlevels) {
        complete_game();
    }
    else {
        clear_console();
        const char *msg = end_level_messages[current_game.level_number - 1];
        int msg_len = strlen(msg);

        putstring(msg, 11,
                  (CONSOLE_WIDTH - msg_len) / 2, (FGND_YLLW | BGND_BLACK));
        putstring(summary_screen_message, 19, 27, (FGND_MAG | BGND_BLACK));

        set_term_color(DEFAULT_COLOR);
        set_cursor(13, 35);
        printf("Moves: %d", current_game.level_moves);

        putstring("Time: ", 14, 35, DEFAULT_COLOR);
        put_time_at_loc(current_game.level_ticks, 14, 41);
    }
}

static void quit_game()
{
    display_introduction();
}

static void pause_game()
{
    current_game.game_state = PAUSED;
    clear_console();
    putstring(pause_screen_message, 12, 30, DEFAULT_COLOR);
}

static void restart_current_level()
{
    current_game.game_state = PAUSED;
    current_game.level_moves = 0;
    current_game.on_goal = false;

    int start_row, start_col, total_boxes;
    if (!draw_sokoban_level(current_game.level, &total_boxes,
                            &start_row, &start_col)) {
        display_introduction();
        return;
    }

    current_game.curr_row = start_row;
    current_game.curr_col = start_col;
    current_game.boxes_left = total_boxes;

    current_game.game_state = RUNNING;
}

static void start_sokoban_level(int level_number)
{
    sokoban.state = LEVEL_RUNNING;

    current_game.level_ticks = 0;
    current_game.level = soko_levels[level_number - 1];
    current_game.level = soko_levels[0];
    current_game.level_number = level_number;

    restart_current_level();
}

static void start_game()
{
    current_game.total_ticks = 0;
    current_game.total_moves = 0;

    start_sokoban_level(1);
}

static void display_instructions()
{
    sokoban.previous_state = sokoban.state;
    sokoban.state = INSTRUCTIONS;
    clear_console();
    const char *ins_str = "Instructions";
    const char *ret_str = "Press 'i' to return";
    putstring(ins_str,
              align_row(TOP_SIDE, STRING_HEIGHT, ALIGNMENT_EIGHT),
              align_col(CENTER, strlen(ins_str), ALIGNMENT_HALF),
              FGND_YLLW | BGND_BLACK);
    putstring(ret_str,
              align_row(BOTTOM_SIDE, STRING_HEIGHT, ALIGNMENT_TENTH),
              align_col(CENTER, strlen(ret_str), ALIGNMENT_HALF),
              FGND_MAG | BGND_BLACK);

    int row = align_row(TOP_SIDE, STRING_HEIGHT, ALIGNMENT_QUARTER);
    int col = align_col(LEFT_SIDE, strlen(ins_str), ALIGNMENT_TWENTYTH);
    int i = 0;
    while (instructions[i] != NULL) {
        putstring(instructions[i], row, col, DEFAULT_COLOR);
        i++;
        row += (STRING_HEIGHT + ELEMENT_ROW_SPACING);
    }
}

static void display_introduction()
{
    sokoban.state = INTRODUCTION;
    clear_console();

    int curr_draw_row;
    int curr_draw_col;

    /* draw the ascii sokoban logo */
    curr_draw_row = align_row(TOP_SIDE, ASCII_SOKO_HEIGHT, ALIGNMENT_TWELFTH);
    curr_draw_col = align_col(CENTER, ASCII_SOKO_WIDTH, ALIGNMENT_HALF);
    draw_image(ascii_sokoban, curr_draw_row, curr_draw_col,
               ASCII_SOKO_HEIGHT, ASCII_SOKO_WIDTH, FGND_YLLW | BGND_BLACK);

    /* draw the author name below the ascii sokoban logo */
    curr_draw_row += (ASCII_SOKO_HEIGHT + ELEMENT_ROW_SPACING);
    curr_draw_col = align_col(CENTER, strlen(name), ALIGNMENT_HALF);
    draw_image(name, curr_draw_row, curr_draw_col,
               STRING_HEIGHT, strlen(name), FGND_MAG | BGND_BLACK);

    /* draw the start message below the author */
    curr_draw_row += (STRING_HEIGHT + ELEMENT_ROW_SPACING);
    curr_draw_col = align_col(CENTER, strlen(intro_screen_message),
                                      ALIGNMENT_HALF);
    draw_image(intro_screen_message, curr_draw_row, curr_draw_col,
               STRING_HEIGHT, strlen(intro_screen_message), DEFAULT_COLOR);

    /* draw the left ascii box 60% from the top and 10% from the left */
    curr_draw_row = align_row(TOP_SIDE, ASCII_LBOX_HEIGHT,
                                   (6 * ALIGNMENT_TENTH));
    curr_draw_col = align_col(LEFT_SIDE, ASCII_LBOX_WIDTH,
                                   (ALIGNMENT_TENTH));
    draw_image(ascii_left_box, curr_draw_row, curr_draw_col,
               ASCII_LBOX_HEIGHT, ASCII_LBOX_WIDTH, FGND_BRWN | BGND_BLACK);

    /* draw the right ascii box 60% from the top and 10% from the right */
    curr_draw_row = align_row(TOP_SIDE, ASCII_RBOX_HEIGHT,
                                   (6 * ALIGNMENT_TENTH));
    curr_draw_col = align_col(RIGHT_SIDE, ASCII_RBOX_WIDTH,
                                   (ALIGNMENT_TENTH));
    draw_image(ascii_right_box, curr_draw_row, curr_draw_col,
               ASCII_RBOX_HEIGHT, ASCII_RBOX_WIDTH, FGND_BRWN | BGND_BLACK);

    /* start drawing high scores at halfway from the top */
    curr_draw_col = align_col(CENTER, strlen("Highscores:"), ALIGNMENT_HALF);
    set_term_color(DEFAULT_COLOR);
    putstring("Highscores:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
    curr_draw_row += ELEMENT_ROW_SPACING;
    curr_draw_col = align_col(LEFT_SIDE, strlen("1 - Moves:"),
                              ALIGNMENT_3EIGHTS);
    int time_draw_col = curr_draw_col + strlen("    Time: ");
    if (sokoban.hiscores[0].num_moves == UINT32_MAX) {
        putstring("1 - Moves:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
        curr_draw_row += ELEMENT_ROW_SPACING;
        putstring("    Time:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
    }
    else {
        set_cursor(curr_draw_row, curr_draw_col);
        printf("1 - Moves: %u", sokoban.hiscores[0].num_moves);
        curr_draw_row += ELEMENT_ROW_SPACING;
        putstring("    Time: ", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
        put_time_at_loc(sokoban.hiscores[0].num_ticks,
                        curr_draw_row, time_draw_col);
    }

    curr_draw_row += ELEMENT_ROW_SPACING;
    if (sokoban.hiscores[1].num_moves == UINT32_MAX) {
        putstring("2 - Moves:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
        curr_draw_row += ELEMENT_ROW_SPACING;
        putstring("    Time:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
    }
    else {
        set_cursor(curr_draw_row, curr_draw_col);
        printf("2 - Moves: %u", sokoban.hiscores[1].num_moves);
        curr_draw_row += ELEMENT_ROW_SPACING;
        putstring("    Time: ", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
        put_time_at_loc(sokoban.hiscores[1].num_ticks,
                        curr_draw_row, time_draw_col);
    }

    curr_draw_row += ELEMENT_ROW_SPACING;
    if (sokoban.hiscores[2].num_moves == UINT32_MAX) {
        putstring("3 - Moves:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
        curr_draw_row += ELEMENT_ROW_SPACING;
        putstring("    Time:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
    }
    else {
        set_cursor(curr_draw_row, curr_draw_col);
        printf("3 - Moves: %u", sokoban.hiscores[2].num_moves);
        curr_draw_row += ELEMENT_ROW_SPACING;
        putstring("    Time: ", curr_draw_row, curr_draw_col, DEFAULT_COLOR);
        put_time_at_loc(sokoban.hiscores[2].num_ticks,
                        curr_draw_row, time_draw_col);
    }
}

void sokoban_initialize_and_run()
{
    score_t default_score = { UINT32_MAX, UINT32_MAX };
    sokoban.hiscores[0] = default_score;
    sokoban.hiscores[1] = default_score;
    sokoban.hiscores[2] = default_score;
    sokoban.state = INTRODUCTION;
    sokoban.previous_state = INTRODUCTION;

    display_introduction();

    int ch;
    while (1) {
        do {
            ch = readchar();
        } while (ch == -1);
        handle_input(ch);
    }
}
