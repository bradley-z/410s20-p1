#include <sokoban_game.h>
#include <stdint.h>
#include <p1kern.h>
#include <video_defines.h>
#include <stdio.h>
#include <string.h>
#include <sokoban.h>

#define ASCII_SPACE 0x20

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

#define CONSOLE_SIZE        (2 * CONSOLE_HEIGHT * CONSOLE_WIDTH)
char saved_screen[CONSOLE_SIZE];
char timer_print_buf[CONSOLE_WIDTH];

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
const char *summary_screen_message = "Press any key to continue";
const char *game_complete_message = "Press any key to return to introduction screen";
const char *pause_screen_message = "Press 'p' to unpause";
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

const char *level_complete_messages[] = {
    "Phase 1 defused. How about the next one?",
    "That's number 2. Keep going!",
    "Halfway there!",
    "So you got that one. Try this one.",
    "Good work! On to the next...",
    "Congratulations! You've defused the bomb! Wait... wrong class...",
};

const char *instructions[] = {
    "0. You are represented by '@', boxes by 'o', and target locations by 'x'",
    "1. Use WASD to move the player onto empty squares",
    "2. You can push boxes onto empty squares and target locations",
    "3. Boxes cannot be pulled, or pushed into other boxes or walls",
    "4. There is an equal number of boxes and target locations",
    "5. Push each box into its own target location to complete the level",
};

game_t current_game;
sokoban_t sokoban;

// TODO: why is my timer half speed
// TODO: clean up code

void sokoban_tickback(unsigned int numTicks)
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

void putstring(const char *str, int row, int col, int color)
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

void start_sokoban_level(int level_number)
{
    sokoban.state = LEVEL_RUNNING;

    current_game.level_ticks = 0;
    // current_game.level = soko_levels[level_number - 1];
    current_game.level = soko_levels[0];
    current_game.level_number = level_number;

    restart_current_level();
}

level_info_t draw_sokoban_level(sokolevel_t *level)
{
    clear_console();

    set_cursor(1, 4);
    printf("Level: %d", current_game.level_number);

    int curr_row = (CONSOLE_HEIGHT - level->height) / 2;
    int curr_col = (CONSOLE_WIDTH - level->width) / 2;
    int first_col = curr_col;
    int end_col = curr_col + level->width - 1;

    int total_pixels = level->height * level->width;
    int pixel_count;

    level_info_t level_info = { 0, -1, -1 };

    for (pixel_count = 0; pixel_count < total_pixels; pixel_count++) {
        char ch = level->map[pixel_count];
        char color;

        switch (ch) {
            case (SOK_WALL):
                color = FGND_DGRAY | BGND_BLACK;
                ch = MY_SOK_WALL;
                break;
            case (SOK_PUSH):
                level_info.start_row = curr_row;
                level_info.start_col = curr_col;
                color = FGND_BCYAN | BGND_BLACK;
                ch = MY_SOK_PLAYER;
                break;
            case (SOK_ROCK):
                level_info.total_boxes++;
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

    int message_row = CONSOLE_HEIGHT - ((CONSOLE_HEIGHT - curr_row) / 2) - 1;
    putstring(game_screen_message, message_row, 4, DEFAULT_COLOR);
    putstring("Moves: ", 3, 4, DEFAULT_COLOR);
    putstring("Time: ", 4, 4, DEFAULT_COLOR);
    print_current_game_moves();
    print_current_game_time();

    return level_info;
}

void print_current_game_moves()
{
    set_cursor(3, 11);
    printf("%d", current_game.level_moves);
}

void print_current_game_time()
{
    // TODO: pull this shit out into another function
    int len = snprintf(timer_print_buf, CONSOLE_WIDTH, "%d", current_game.level_ticks / 10);
    timer_print_buf[len + 1] = '\0';
    timer_print_buf[len] = timer_print_buf[len - 1];
    timer_print_buf[len - 1] = '.';
    putstring(timer_print_buf, 4, 10, DEFAULT_COLOR);
}

void complete_level()
{
    current_game.game_state = IN_LEVEL_SUMMARY;

    current_game.total_ticks += current_game.level_ticks;
    current_game.total_moves += current_game.level_moves;
    if (current_game.level_number == soko_nlevels) {
        complete_game();
    }
    else {
        clear_console();
        const char *msg = level_complete_messages[current_game.level_number - 1];
        int msg_len = strlen(msg);

        putstring(msg, 11, (CONSOLE_WIDTH - msg_len) / 2, (FGND_YLLW | BGND_BLACK));
        putstring(summary_screen_message, 19, 27, (FGND_MAG | BGND_BLACK));

        set_term_color(DEFAULT_COLOR);
        set_cursor(13, 35);
        printf("Moves: %d", current_game.level_moves);

        int len = snprintf(timer_print_buf, CONSOLE_WIDTH, "Time: %d", current_game.level_ticks / 10);
        timer_print_buf[len + 1] = '\0';
        timer_print_buf[len] = timer_print_buf[len - 1];
        timer_print_buf[len - 1] = '.';
        putstring(timer_print_buf, 14, 35, DEFAULT_COLOR);
    }
}

void try_move(dir_t dir)
{
    int row = current_game.curr_row;
    int col = current_game.curr_col;

    int new_row, new_col;

    switch (dir) {
        case UP:
            if (row - 1 < 0) {
                return;
            }
            new_row = row - 1;
            new_col = col;
            break;
        case DOWN:
            if (row + 1 >= CONSOLE_HEIGHT) {
                return;
            }
            new_row = row + 1;
            new_col = col;
            break;
        case LEFT:
            if (col - 1 < 0) {
                return;
            }
            new_row = row;
            new_col = col - 1;
            break;
        case RIGHT:
            if (col + 1 >= CONSOLE_WIDTH - 1) {
                return;
            }
            new_row = row;
            new_col = col + 1;
            break;
        default:
            return;
    }

    char next_square = get_char(new_row, new_col);
    
    if (next_square == ASCII_SPACE) {
        if (current_game.on_goal) {
            draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
            current_game.on_goal = false;
        }
        else {
            draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
        }
        draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
        current_game.level_moves++;
        current_game.curr_row = new_row;
        current_game.curr_col = new_col;
    }
    else if (next_square == MY_SOK_GOAL) {
        if (current_game.on_goal) {
            draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
        }
        else {
            draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
            current_game.on_goal = true;
        }
        draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
        current_game.level_moves++;
        current_game.curr_row = new_row;
        current_game.curr_col = new_col;
    }
    else if (next_square == MY_SOK_BOX) {
        int next_next_square_row, next_next_square_col;

        switch (dir) {
            case UP:
                if (row - 2 < 0) {
                    return;
                }
                next_next_square_row = row - 2;
                next_next_square_col = col;
                break;
            case DOWN:
                if (row + 2 >= CONSOLE_HEIGHT) {
                    return;
                }
                next_next_square_row = row + 2;
                next_next_square_col = col;
                break;
            case LEFT:
                if (col - 2 < 0) {
                    return;
                }
                next_next_square_row = row;
                next_next_square_col = col - 2;
                break;
            case RIGHT:
                if (col + 2 >= CONSOLE_WIDTH) {
                    return;
                }
                next_next_square_row = row;
                next_next_square_col = col + 2;
                break;
            default:
                return;
        }

        char next_next_square = get_char(next_next_square_row, next_next_square_col);

        if (next_next_square == ASCII_SPACE) {
            if (current_game.on_goal) {
                draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
                current_game.on_goal = false;
            }
            else {
                draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
            }
            draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
            draw_char(next_next_square_row, next_next_square_col, MY_SOK_BOX, BOX_COLOR);
            current_game.level_moves++;
            current_game.curr_row = new_row;
            current_game.curr_col = new_col;
        }
        else if (next_next_square == MY_SOK_GOAL) {
            if (current_game.on_goal) {
                draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
                current_game.on_goal = false;
            }
            else {
                draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
            }
            draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
            draw_char(next_next_square_row, next_next_square_col, MY_SOK_BOX_ON_GOAL, BOX_ON_GOAL_COLOR);
            current_game.level_moves++;
            current_game.boxes_left--;
            current_game.curr_row = new_row;
            current_game.curr_col = new_col;
        }
    }
    else if (next_square == MY_SOK_BOX_ON_GOAL) {
        int next_next_square_row, next_next_square_col;

        switch (dir) {
            case UP:
                if (row - 2 < 0) {
                    return;
                }
                next_next_square_row = row - 2;
                next_next_square_col = col;
                break;
            case DOWN:
                if (row + 2 >= CONSOLE_HEIGHT) {
                    return;
                }
                next_next_square_row = row + 2;
                next_next_square_col = col;
                break;
            case LEFT:
                if (col - 2 < 0) {
                    return;
                }
                next_next_square_row = row;
                next_next_square_col = col - 2;
                break;
            case RIGHT:
                if (col + 2 >= CONSOLE_WIDTH) {
                    return;
                }
                next_next_square_row = row;
                next_next_square_col = col + 2;
                break;
            default:
                return;
        }

        char next_next_square = get_char(next_next_square_row, next_next_square_col);

        if (next_next_square == ASCII_SPACE) {
            if (current_game.on_goal) {
                draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
            }
            else {
                draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
                current_game.on_goal = true;
            }
            draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
            draw_char(next_next_square_row, next_next_square_col, MY_SOK_BOX, BOX_COLOR);
            current_game.level_moves++;
            current_game.boxes_left++;
            current_game.curr_row = new_row;
            current_game.curr_col = new_col;
        }
        else if (next_next_square == MY_SOK_GOAL) {
            if (current_game.on_goal) {
                draw_char(row, col, MY_SOK_GOAL, GOAL_COLOR);
            }
            else {
                draw_char(row, col, ASCII_SPACE, PLAYER_COLOR);
                current_game.on_goal = true;
            }
            draw_char(new_row, new_col, MY_SOK_PLAYER, PLAYER_COLOR);
            draw_char(next_next_square_row, next_next_square_col, MY_SOK_BOX_ON_GOAL, BOX_ON_GOAL_COLOR);
            current_game.level_moves++;
            current_game.curr_row = new_row;
            current_game.curr_col = new_col;
        }
    }
    print_current_game_moves();

    if (current_game.boxes_left == 0) {
        complete_level();
    }
}

void handle_input(char ch)
{
    if (sokoban.state == LEVEL_RUNNING && current_game.game_state == IN_LEVEL_SUMMARY) {
        level_up();
        return;
    }

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
        case 'q':
            if (sokoban.state == LEVEL_RUNNING && current_game.game_state == RUNNING) {
                quit_game();
            }
            break;
        case 'r':
            if (sokoban.state == LEVEL_RUNNING && current_game.game_state == RUNNING) {
                restart_current_level();
            }
            break;
        case 'w':
            if (sokoban.state == LEVEL_RUNNING && current_game.game_state == RUNNING) {
                try_move(UP);
            }
            break;
        case 's':
            if (sokoban.state == LEVEL_RUNNING && current_game.game_state == RUNNING) {
                try_move(DOWN);
            }
            break;
        case 'a':
            if (sokoban.state == LEVEL_RUNNING && current_game.game_state == RUNNING) {
                try_move(LEFT);
            }
            break;
        case 'd':
            if (sokoban.state == LEVEL_RUNNING && current_game.game_state == RUNNING) {
                try_move(RIGHT);
            }
            break;
        default:
            break;
    }
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

void level_up()
{
    if (current_game.level_number == soko_nlevels) {
        display_introduction();
    }
    else {
        current_game.level_number++;
        start_sokoban_level(current_game.level_number);
    }
}

void complete_game()
{
    score_t score = { current_game.total_moves, current_game.total_ticks / 10 };

    int i;
    for (i = 0; i < 3; i++) {
        if (score.num_moves < sokoban.hiscores[i].num_moves ||
           (score.num_moves == sokoban.hiscores[i].num_moves && score.num_ticks < sokoban.hiscores[i].num_ticks)) {
            int j;
            for (j = 2; j > i; j--) {
                sokoban.hiscores[j] = sokoban.hiscores[j - 1];
            }
            sokoban.hiscores[i] = score;
            break;
        }
    }

    clear_console();
    const char *msg = level_complete_messages[current_game.level_number - 1];
    int msg_len = strlen(msg);

    putstring(msg, 11, (CONSOLE_WIDTH - msg_len) / 2, FGND_YLLW | BGND_BLACK);
    putstring(game_complete_message, 19, 17, FGND_MAG | BGND_BLACK);

    set_term_color(DEFAULT_COLOR);
    set_cursor(13, 32);
    printf("Total moves: %d", current_game.total_moves);
    set_cursor(14, 32);
    int len = snprintf(timer_print_buf, CONSOLE_WIDTH, "Total time: %u", current_game.total_ticks / 10);
    timer_print_buf[len + 1] = '\0';
    timer_print_buf[len] = timer_print_buf[len - 1];
    timer_print_buf[len - 1] = '.';
    putstring(timer_print_buf, 14, 32, DEFAULT_COLOR);
}

void quit_game()
{
    display_introduction();
}

void restart_current_level()
{
    current_game.game_state = PAUSED;
    current_game.level_moves = 0;
    current_game.on_goal = false;

    level_info_t level_info = draw_sokoban_level(current_game.level);

    current_game.curr_row = level_info.start_row;
    current_game.curr_col = level_info.start_col;
    current_game.boxes_left = level_info.total_boxes;

    current_game.game_state = RUNNING;
}

void pause_game()
{
    current_game.game_state = PAUSED;
    clear_console();
    putstring(pause_screen_message, 12, 30, DEFAULT_COLOR);
}

void start_game()
{
    sokoban.previous_state = sokoban.state;

    current_game.total_ticks = 0;
    current_game.total_moves = 0;

    start_sokoban_level(1);
}

void display_instructions()
{
    sokoban.previous_state = sokoban.state;
    sokoban.state = INSTRUCTIONS;
    clear_console();
    putstring("Instructions", 4, 34, FGND_YLLW | BGND_BLACK);
    putstring("Press 'i' to return", 20, 31, FGND_MAG | BGND_BLACK);

    int row = 7;
    int i;
    for (i = 0; i < 5; i++) {
        putstring(instructions[i], row, 3, DEFAULT_COLOR);
        row += 2;
    }
}

void display_introduction()
{
    sokoban.previous_state = sokoban.state;
    sokoban.state = INTRODUCTION;
    clear_console();
    draw_image(2, 18, 6, 43, FGND_YLLW | BGND_BLACK, ascii_sokoban);
    draw_image(9, 28, 1, 23, FGND_MAG | BGND_BLACK, name);
    draw_image(11, 17, 1, 46, DEFAULT_COLOR, intro_screen_message);
    draw_image(15, 9, 7, 13, FGND_BRWN | BGND_BLACK, ascii_left_box);
    draw_image(15, 58, 7, 12, FGND_BRWN | BGND_BLACK, ascii_right_box);

    int buf_len;

    set_term_color(DEFAULT_COLOR);
    putstring("Highscores:", 15, 34, DEFAULT_COLOR);
    if (sokoban.hiscores[0].num_moves == UINT32_MAX) {
        putstring("1 - Moves:", 16, 30, DEFAULT_COLOR);
        putstring("Time:", 17, 34, DEFAULT_COLOR);
    }
    else {
        set_cursor(16, 30);
        printf("1 - Moves: %u", sokoban.hiscores[0].num_moves);
        buf_len = snprintf(timer_print_buf, CONSOLE_WIDTH, "Time: %u", sokoban.hiscores[0].num_ticks);
        timer_print_buf[buf_len + 1] = '\0';
        timer_print_buf[buf_len] = timer_print_buf[buf_len - 1];
        timer_print_buf[buf_len - 1] = '.';
        putstring(timer_print_buf, 17, 34, DEFAULT_COLOR);
    }

    if (sokoban.hiscores[1].num_moves == UINT32_MAX) {
        putstring("2 - Moves:", 18, 30, DEFAULT_COLOR);
        putstring("Time:", 19, 34, DEFAULT_COLOR);
    }
    else {
        set_cursor(18, 30);
        printf("2 - Moves: %u", sokoban.hiscores[1].num_moves);

        buf_len = snprintf(timer_print_buf, CONSOLE_WIDTH, "Time: %u", sokoban.hiscores[1].num_ticks);
        timer_print_buf[buf_len + 1] = '\0';
        timer_print_buf[buf_len] = timer_print_buf[buf_len - 1];
        timer_print_buf[buf_len - 1] = '.';
        putstring(timer_print_buf, 19, 34, DEFAULT_COLOR);
    }

    if (sokoban.hiscores[2].num_moves == UINT32_MAX) {
        putstring("3 - Moves:", 20, 30, DEFAULT_COLOR);
        putstring("Time:", 21, 34, DEFAULT_COLOR);
    }
    else {
        set_cursor(20, 30);
        printf("3 - Moves: %u", sokoban.hiscores[2].num_moves);

        buf_len = snprintf(timer_print_buf, CONSOLE_WIDTH, "Time: %u", sokoban.hiscores[2].num_ticks);
        timer_print_buf[buf_len + 1] = '\0';
        timer_print_buf[buf_len] = timer_print_buf[buf_len - 1];
        timer_print_buf[buf_len - 1] = '.';
        putstring(timer_print_buf, 21, 34, DEFAULT_COLOR);
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