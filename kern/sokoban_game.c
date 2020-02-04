/** @file sokoban_game.c
 *  @brief game implementation
 *
 *  This contains all the implementation details for the sokoban game. Detailed
 *  functionality explanations and design choices are documented in README.dox.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug No known bugs.
 */
#include <sokoban_game.h>
#include <p1kern.h>
#include <sokoban.h>
#include <stdbool.h>        /* bool */
#include <stdint.h>         /* UINT32_MAX */
#include <video_defines.h>  /* console size, color constants */
#include <stdio.h>          /* printf() */
#include <string.h>         /* memcpy() */

/* scoring system is just moves/time, so default score is just the max val */
#define DEFAULT_SCORE       UINT32_MAX

/* ASCII code for space character */
#define ASCII_SPACE         0x20

/* I like these symbols better (ones shown in handout) */
#define MY_SOK_WALL         ((char)0xB0)
#define MY_SOK_PLAYER       ('@')
#define MY_SOK_BOX          ('o')
#define MY_SOK_GOAL         ('x')
/* Distinguish between boxes on goal and boxes not on goal in game logic */
#define MY_SOK_BOX_ON_GOAL  ('O')

/* Lakers colors bc rip kobe :'( */
#define MAIN_COLOR          (FGND_YLLW | BGND_BLACK)
#define ACCENT_COLOR        (FGND_MAG | BGND_BLACK)

/* Colors to draw the level elements and also text */
#define DEFAULT_COLOR       (FGND_WHITE | BGND_BLACK)
#define WALL_COLOR          (FGND_DGRAY | BGND_BLACK)
#define PLAYER_COLOR        (FGND_BCYAN | BGND_BLACK)
#define BOX_COLOR           (FGND_BRWN | BGND_BLACK)
#define GOAL_COLOR          (FGND_YLLW | BGND_BLACK)
#define BOX_ON_GOAL_COLOR   (FGND_GREEN | BGND_BLACK)

/* (rounded) Percentages for my align functions */
#define ALIGNMENT_TWENTYTH  5
#define ALIGNMENT_TWELFTH   8
#define ALIGNMENT_TENTH     10
#define ALIGNMENT_EIGHT     12
#define ALIGNMENT_SIXTH     17
#define ALIGNMENT_FIFTH     20
#define ALIGNMENT_QUARTER   25
#define ALIGNMENT_THIRD     33
#define ALIGNMENT_3EIGHTS   38
#define ALIGNMENT_HALF      50
#define MAX_PERCENT         100

/* Rows to print level, num moves, and time at; independent of screen size */
#define LEVEL_INFO_ROW      1
#define MOVES_INFO_ROW      3
#define TIME_INFO_ROW       4
#define SIDE_INFO_COL       4

/* Constants to define the sizes of my beautiful ASCII art images */
#define ASCII_SOKO_HEIGHT   6
#define ASCII_SOKO_WIDTH    43
#define ASCII_LBOX_HEIGHT   7
#define ASCII_LBOX_WIDTH    13
#define ASCII_RBOX_HEIGHT   7
#define ASCII_RBOX_WIDTH    12

/* Strings can be considered images of width strlen() and height 1 */
#define STRING_HEIGHT       1
/* Spacing between drawn elements on the screen */
#define ELEMENT_ROW_SPACING 1

/**
 *  This constant be kinda jank; Certain alignment features require centering by
 *  string length, but the way I print time doesn't actually include all the
 *  characters that should be printed, since those are printed with the
 *  print_current_game_time() function. This is just an offset that accounts
 *  for those extra characters to achieve proper alignment.
 */
#define FORMAT_STR_OFFSET   3

/* Size of the console screen in bytes */
#define CONSOLE_SIZE        (2 * CONSOLE_HEIGHT * CONSOLE_WIDTH)

/** @brief used to align an image vertically
 *
 *  This alignment function isn't the most intuitive. Here is an example of its
 *  usage: we want to draw an image that's 8 rows tall. If we want to align the
 *  center of the image with the center of the console, alignment would be
 *  CENTER and percentage would be 50. If we want to align the top side of the
 *  image with a quarter of the way down the console, alignment would be
 *  TOP_SIDE and percentage would be 25. The last possible alignment for this
 *  function is BOTTOM_SIDE. In this case, the alignment percentage is offset
 *  from the BOTTOM of the console. Therefore, parameters BOTTOM_SIDE, 8, 25
 *  would align the bottom of the image with a quarter of the way UP the console
 *  and then return the row to start drawing the top of the image from. If the
 *  alignment is not one of these, or the image would be out of range, a
 *  negative number is returned. Otherwise, the number returned is the row that
 *  one should start drawing at to get the desired alignment.
 *
 *  @param alignment from what part of the image we are aligning
 *  @param height how many rows does the image take up
 *  @param percentage how far vertically we want to align the image to
 *  @return row to start drawing the image at
 */
static inline int align_row(alignment_t alignment, int height, int percentage);
/** @brief used to align an image horizontally
 *
 *  Analogous to the previous function. Possible alignments for this function
 *  are LEFT_SIDE, CENTER, and RIGHT_SIDE. LEFT_SIDE and RIGHT_SIDE behave
 *  similarly to TOP_SIDE and BOTTOM_SIDE, respectively.
 *
 *  @param alignment from what part of the image we are aligning
 *  @param width how many columns does the image take up
 *  @param percentage how far horizontally we want to align the image to
 *  @return column to start drawing the image at
 */
static inline int align_col(alignment_t alignment, int width, int percentage);

/** @brief draws an image (represented by a string) at a given row/column
 *
 *  This function doesn't need to clear the console because its purpose is to
 *  draw an image in addition to what's already on the screen.
 *
 *  To be honest, this is mostly just used for my beautiful ASCII art.
 *
 *  @param image image to draw (represented by a string of len (height * width))
 *  @param start_row row to start drawing at
 *  @param start_col col to start drawing at
 *  @param height how many rows the image takes up
 *  @param width how many columns the image takes up
 *  @param color what color to print the image
 *  @return Void.
 */
static void draw_image(const char *image, int start_row, int start_col,
                       int height, int width, int color);
/** @brief draws the specified level and checks if it's valid
 *
 *  Centers the map of the level and iterates through the string, printing
 *  symbols accordingly. While iterating through, we also check to make sure the
 *  map is valid, return true if it is and false otherwise. We also draw the
 *  level number/num moves/time information.
 *
 *  Things that are invalid:
 *  Any NULL parameters
 *  Zero boxes found
 *  No player character found
 *  Multiple player characters found
 *
 *  @param level level to draw
 *  @param total_boxes pointer to int to store total number of boxes at
 *  @param start_row pointer to int to store the player's start row at
 *  @param start_col pointer to int to store the player's start column at
 *  @return whether or not the level is valid
 */
static bool draw_sokoban_level(sokolevel_t *level, int *total_boxes,
                               int *start_row, int *start_col);
/** @brief prints the current time at specified location
 *
 *  I wanted to print time in 0.1 second intervals. Since we have a tick every
 *  10 ms, there are 10 ticks in ever 0.1 second interval. snprintf() is used to
 *  print the number of ticks / 10 into a buffer and the return value of
 *  snprintf() is used to add a '.' before the last number to emulate having
 *  0.1 second floating point precision.
 *
 *  @param ticks number of ticks to print time representation of
 *  @param row row to print time at
 *  @param col column to print time at
 *  @return Void.
 */
static void put_time_at_loc(int ticks, int row, int col);
/** @brief prints the number of moves in current level
 *
 *  Prints the current number of moves in the level at the fixed moves location
 *  at the top left of the screen. Called after we make a move and as we
 *  start/restart a level.
 *
 *  @return Void.
 */
static void print_current_game_moves(void);
/** @brief prints time elapsed in current level
 *
 *  Prints the amount of time that has elapsed while the current level running
 *  has not been paused. Called as we start/restart a level as well as during
 *  the tickback function.
 *
 *  @return Void.
 */
static void print_current_game_time(void);
/** @brief prints a string at a given row/col with a given color
 *
 *  One pass method to printing a string that we don't know the length of as
 *  well as in a different color from the console wide one. Not super necessary
 *  since printing unknown strings is potentially unsafe and printing known
 *  const char* strings, we can just use putbytes() since strlen() optimizes
 *  down to a constant. However, it wraps up the logic to printing at a specific
 *  position/color nicely.
 *
 *  @param str string to print
 *  @param row row to start printing at
 *  @param col column to start printing at
 *  @param color color to print the string
 *  @return Void.
 */
static void putstring(const char *str, int row, int col, int color);

/** @brief checks to see if the next square in the given direction is in range
 *
 *  Used to modularize moving in any direction while simultaneously checking
 *  if it's in range of the console.
 *
 *  @param dir direction to look in
 *  @param row row we're currently at
 *  @param col column we're currently at
 *  @param new_row pointer to int we'll store the next square's row at
 *  @param new_col pointer to int we'll store the next square's column at
 *  @return whether or not the next square in the given direction is in range.
 */
static bool valid_next_square(dir_t dir, int row, int col,
                              int *new_row, int *new_col);
/** @brief attempts to move in the given direction
 *
 *  This function contains most of the actual game logic (which really only
 *  consists of movement logic). Logic breakdown:
 *  
 *  Check the validity and symbol of the next square in the given direction.
 *  If it is a space or a goal, we can move onto it, drawing our player symbol
 *  at the new square. As we leave, we check to see if were on a goal before,
 *  drawing the appropriate symbols. Additionally, we update our "on_goal"
 *  status accordingly if we moved off of or onto a goal. Finally, we update the
 *  amount of moves and our position.
 *
 *  If it is a box or a box on a goal, we can push it, but we have to check the
 *  square after it first. If the square after the box is a space or a goal, we
 *  can push the box. Again, as we leave, we check to see if we were on a goal
 *  before, drawing the appropriate symbols. Additionally, we need to update
 *  statuses based on whether the box we are pushing was previously on a goal or
 *  not. Finally, we need to update based on whether the square we're pushing
 *  the box onto is a space or a goal. We update "boxes_left" based on this
 *  information as well as the "on_goal" and the moves and position.
 *
 *  Finally, we check the number of boxes we have left and if it's 0, we call
 *  the complete_level() function.
 *
 *  All of these squares are checked to see if they are in range. Additionally,
 *  in the case that a particular move cannot be made or a box cannot be pushed,
 *  the the function simply returns without moving anything.
 *
 *  @param dir direction we're trying to move in
 *  @return Void.
 */
static void try_move(dir_t dir);
/** @brief checks input character and handles it accordingly
 *
 *  There is a different set of valid keypresses depending on the state of the
 *  game. For the most part, this function just falls into case statements to
 *  call an appropriate function, but certain keys (pause/instructions) will
 *  also save the state of the current screen before transitioning. When the
 *  appropriate keypress is made to leave the pause/instructions, the old screen
 *  is restored.
 *
 *  @param ch input keypress
 *  @return Void.
 */
static void handle_input(char ch);
/** @brief shows the next screen after we complete a level
 *
 *  Called after we press any key in a level summary. If we just completed our
 *  final level, we go back to introduction screen. Otherwise, we increment the
 *  level of our game and start the new level.
 *
 *  @return Void.
 */
static void level_up(void);
/** @brief prints the level summary screen and waits for any key press
 *
 *  Displays end level message, moves, time, and changes game state so
 *  handle_input() knows to expect any key. If we've completed the last level,
 *  update the highscores accordingly as well.
 *
 *  @return Void.
 */
static void complete_level(void);
/** @brief quits the actively running game
 *
 *  Can only be called if the current level is running. This just returns to
 *  introduction screen.
 *
 *  @return Void.
 */
static void quit_game(void);
/** @brief pauses the actively running game
 *
 *  Can only be called if the current level is running. Changes game state,
 *  clears console, and displays the paused message.
 *
 *  @return Void.
 */
static void pause_game(void);
/** @brief restarts the actively running game
 *
 *  Can only be called if the current level is running (and also while
 *  initializing a level for the first time). Resets the level_moves to 0 and
 *  draws the current game level. We set the game state to PAUSED so the timer
 *  interrupt handler doesn't increase the tick count before we're ready. If the
 *  map is invalid, just return to introduction screen.
 *
 *  We don't want to reset the level_ticks to 0 because we still want to keep
 *  track of the total time the player has spent on the level.
 *
 *  @return Void.
 */
static void restart_current_level(void);
/** @brief starts a sokoban level
 *
 *  Sets game states and level data. We also initialize level_ticks to 0 here
 *  because we want it to increment regardless of how many times the level has
 *  been reset; otherwise, the user could just reset the level to reset their
 *  time.
 *
 *  @param level_number level number to start (not zero indexed)
 *  @return Void.
 */
static void start_sokoban_level(int level_number);
/** @brief starts a sokoban game
 *
 *  For clarification for my terminology, a sokoban game consists of playing
 *  through all of the sokoban levels. This function resets the total_ticks and
 *  total_moves to 0 and starts the first level.
 *
 *  @return Void.
 */
static void start_game(void);
/** @brief displays instructions screen
 *
 *  Loops through instructions string array and displays them one by one.
 *
 *  @return Void.
 */
static void display_instructions(void);
/** @brief displays introduction screen
 *
 *  Changes state then does a lot of weird alignment to draw my beautiful ASCII
 *  arts as well as the highscores. Don't print default highscores.
 *
 *  @return Void.
 */
static void display_introduction(void);

/* ASCII art used in my introduction screen */
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

/* constant strings that are utilized in the various game screens */
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

/* state of the screen before we pause/instructions for easy recovery */
char saved_screen[CONSOLE_SIZE];
/* intermediate buffer to create our 0.1 second precision timing */
char timer_print_buf[CONSOLE_WIDTH];

/* state of the currently running sokoban game; not looked at if not running */
game_t current_game;
/* metadata of sokoban game */
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
            return (CONSOLE_HEIGHT -
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
            return (CONSOLE_WIDTH -
                    (CONSOLE_WIDTH * percentage / MAX_PERCENT) -
                    width);
        default:
            return -1;
    }
}

void sokoban_tickback()
{
    if (sokoban.state != GAME_RUNNING) {
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
    if (level == NULL || total_boxes == NULL ||
        start_row == NULL || start_col == NULL) {
        return false;
    }

    clear_console();

    set_cursor(LEVEL_INFO_ROW, SIDE_INFO_COL);
    printf("Level: %d", current_game.level_number);

    int curr_row = align_row(CENTER, level->height, ALIGNMENT_HALF);
    int first_col = align_col(CENTER, level->width, ALIGNMENT_HALF);
    int curr_col = first_col;
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
                color = WALL_COLOR;
                ch = MY_SOK_WALL;
                break;
            case (SOK_PUSH):
                /* multiple starting positions */
                if (found_start == true) {
                    return false;
                }
                *start_row = curr_row;
                *start_col = curr_col;
                color = PLAYER_COLOR;
                ch = MY_SOK_PLAYER;
                found_start = true;
                break;
            case (SOK_ROCK):
                num_boxes++;
                color = BOX_COLOR;
                ch = MY_SOK_BOX;
                break;
            case (SOK_GOAL):
                color = GOAL_COLOR;
                ch = MY_SOK_GOAL;
                break;
            default:
                /* space character */
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
            /* wrap back around */
            curr_col = first_col;
            curr_row++;
        }
        else {
            curr_col++;
        }
    }

    /* if there are no boxes or no starting position, board is invalid */
    if (num_boxes == 0 || !found_start) {
        return false;
    }

    *total_boxes = num_boxes;

    /* print game information */
    int message_row = align_row(BOTTOM_SIDE, STRING_HEIGHT, ALIGNMENT_SIXTH);
    int message_col = align_col(CENTER,
                                strlen(game_screen_message), ALIGNMENT_HALF);
    putstring(game_screen_message, message_row, message_col, DEFAULT_COLOR);
    putstring("Moves: ", MOVES_INFO_ROW, SIDE_INFO_COL, DEFAULT_COLOR);
    putstring("Time: ", TIME_INFO_ROW, SIDE_INFO_COL, DEFAULT_COLOR);
    print_current_game_moves();
    print_current_game_time();

    return true;
}

static void put_time_at_loc(int ticks, int row, int col)
{
    /* uses return value of snprintf to know where to draw decimal point */
    int len = snprintf(timer_print_buf, CONSOLE_WIDTH, "%d", ticks / 10);
    timer_print_buf[len + 1] = '\0';
    timer_print_buf[len] = timer_print_buf[len - 1];
    timer_print_buf[len - 1] = '.';
    /* display string once it has been formed */
    putstring(timer_print_buf, row, col, DEFAULT_COLOR);
}

static void print_current_game_moves()
{
    set_cursor(MOVES_INFO_ROW, SIDE_INFO_COL);
    printf("Moves: %d", current_game.level_moves);
}

static void print_current_game_time()
{
    put_time_at_loc(current_game.level_ticks,
                    /* SIDE_INFO_COL is where the string "Time: " starts */
                    TIME_INFO_ROW, SIDE_INFO_COL + strlen("Time: "));
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
            /* if we were on a goal and move onto a goal, on_goal still true */
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
        /* square after the box */
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
                else if (sokoban.previous_state == GAME_RUNNING) {
                    memcpy((void*)CONSOLE_MEM_BASE,
                           (void*)saved_screen, CONSOLE_SIZE);
                    sokoban.state = GAME_RUNNING;
                    current_game.game_state = RUNNING;
                }
                break;
            default:
                return;
        }
    }
    else if (state == GAME_RUNNING) {
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

static void complete_level()
{
    current_game.game_state = IN_LEVEL_SUMMARY;

    current_game.total_ticks += current_game.level_ticks;
    current_game.total_moves += current_game.level_moves;

    clear_console();
    const char *msg = end_level_messages[current_game.level_number - 1];

    const char *end_level_msg = summary_screen_message;
    const char *moves_fmt = "Moves: %d";
    const char *time_fmt = "Time: ";
    int moves = current_game.level_moves;
    unsigned int ticks = current_game.level_ticks;

    /* love me my alignment */
    int forty_percent = 4 * ALIGNMENT_TENTH;

    putstring(msg,
              align_row(TOP_SIDE, STRING_HEIGHT, forty_percent),
              align_col(CENTER, strlen(msg), ALIGNMENT_HALF),
              MAIN_COLOR);

    /* save highscores and change string to display total moves/ticks */
    if (current_game.level_number == soko_nlevels) {
        score_t score = { current_game.total_moves, current_game.total_ticks };

        int i;
        for (i = 0; i < NUM_HIGHSCORES; i++) {
            if (score.num_moves < sokoban.highscores[i].num_moves ||
               (score.num_moves == sokoban.highscores[i].num_moves &&
                score.num_ticks < sokoban.highscores[i].num_ticks)) {
                int j;
                for (j = (NUM_HIGHSCORES - 1); j > i; j--) {
                    sokoban.highscores[j] = sokoban.highscores[j - 1];
                }
                sokoban.highscores[i] = score;
                break;
            }
        }
        end_level_msg = game_complete_message;
        moves_fmt = "Total moves: %d";
        time_fmt = "Total time: ";
        moves = current_game.total_moves;
        ticks = current_game.total_ticks;
    }

    /* display message and moves/time information */
    putstring(end_level_msg,
              align_row(BOTTOM_SIDE, STRING_HEIGHT, ALIGNMENT_QUARTER),
              align_col(CENTER, strlen(end_level_msg), ALIGNMENT_HALF),
              ACCENT_COLOR);
    int moves_row = align_row(BOTTOM_SIDE, STRING_HEIGHT, ALIGNMENT_HALF);
    int moves_col = align_col(CENTER, strlen(moves_fmt), ALIGNMENT_HALF);
    int time_row = moves_row + ELEMENT_ROW_SPACING;
    int time_col = align_col(CENTER, strlen(time_fmt) + FORMAT_STR_OFFSET,
                             ALIGNMENT_HALF);
    int time_tick_col = time_col + strlen(time_fmt);
    set_cursor(moves_row, moves_col);
    printf(moves_fmt, moves);
    putstring(time_fmt, time_row, time_col, DEFAULT_COLOR);
    put_time_at_loc(ticks, time_row, time_tick_col);
}

static void quit_game()
{
    display_introduction();
}

static void pause_game()
{
    current_game.game_state = PAUSED;
    clear_console();
    putstring(pause_screen_message,
              align_row(TOP_SIDE, STRING_HEIGHT, ALIGNMENT_HALF),
              align_col(CENTER, strlen(pause_screen_message), ALIGNMENT_HALF),
              DEFAULT_COLOR);
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
    current_game.game_state = PAUSED;
    sokoban.state = GAME_RUNNING;

    current_game.level_ticks = 0;
    current_game.level = soko_levels[level_number - 1];
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
              MAIN_COLOR);
    putstring(ret_str,
              align_row(BOTTOM_SIDE, STRING_HEIGHT, ALIGNMENT_TENTH),
              align_col(CENTER, strlen(ret_str), ALIGNMENT_HALF),
              ACCENT_COLOR);

    int row = align_row(TOP_SIDE, STRING_HEIGHT, ALIGNMENT_QUARTER);
    int col = align_col(LEFT_SIDE, strlen(ins_str), ALIGNMENT_TWENTYTH);
    int i = 0;
    /* display each instruction on its own line */
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
               ASCII_SOKO_HEIGHT, ASCII_SOKO_WIDTH, MAIN_COLOR);

    /* draw the author name below the ascii sokoban logo */
    curr_draw_row += (ASCII_SOKO_HEIGHT + ELEMENT_ROW_SPACING);
    curr_draw_col = align_col(CENTER, strlen(name), ALIGNMENT_HALF);
    draw_image(name, curr_draw_row, curr_draw_col,
               STRING_HEIGHT, strlen(name), ACCENT_COLOR);

    /* draw the start message below the author */
    curr_draw_row += (STRING_HEIGHT + ELEMENT_ROW_SPACING);
    curr_draw_col = align_col(CENTER, strlen(intro_screen_message),
                                      ALIGNMENT_HALF);
    draw_image(intro_screen_message, curr_draw_row, curr_draw_col,
               STRING_HEIGHT, strlen(intro_screen_message), DEFAULT_COLOR);


    int sixty_percent = 6 * ALIGNMENT_TENTH;
    /* draw the left ascii box 60% from the top and 10% from the left */
    curr_draw_row = align_row(TOP_SIDE, ASCII_LBOX_HEIGHT, sixty_percent);
    curr_draw_col = align_col(LEFT_SIDE, ASCII_LBOX_WIDTH, ALIGNMENT_TENTH);
    draw_image(ascii_left_box, curr_draw_row, curr_draw_col,
               ASCII_LBOX_HEIGHT, ASCII_LBOX_WIDTH, BOX_COLOR);

    /* draw the right ascii box 60% from the top and 10% from the right */
    curr_draw_row = align_row(TOP_SIDE, ASCII_RBOX_HEIGHT, sixty_percent);
    curr_draw_col = align_col(RIGHT_SIDE, ASCII_RBOX_WIDTH, ALIGNMENT_TENTH);
    draw_image(ascii_right_box, curr_draw_row, curr_draw_col,
               ASCII_RBOX_HEIGHT, ASCII_RBOX_WIDTH, BOX_COLOR);

    /* draw highscores wording in line with top of the boxes */
    curr_draw_col = align_col(CENTER, strlen("Highscores:"), ALIGNMENT_HALF);
    set_term_color(DEFAULT_COLOR);
    putstring("Highscores:", curr_draw_row, curr_draw_col, DEFAULT_COLOR);

    /* start drawing the actual highscores right under that */
    curr_draw_row += ELEMENT_ROW_SPACING;
    const char *moves_fmt = "%d - Moves: ";
    const char *time_str = "    Time: ";
    curr_draw_col = align_col(LEFT_SIDE, strlen(moves_fmt), ALIGNMENT_3EIGHTS);
    int time_draw_col = curr_draw_col + strlen(time_str);

    int i;
    for (i = 0; i < NUM_HIGHSCORES; i++) {
        set_cursor(curr_draw_row, curr_draw_col);
        printf(moves_fmt, i + 1);
        /* only print moves/time if it's not default */
        if (sokoban.highscores[i].num_moves != DEFAULT_SCORE) {
            printf("%u", sokoban.highscores[i].num_moves);
        }
        curr_draw_row += ELEMENT_ROW_SPACING;
        putstring(time_str, curr_draw_row, curr_draw_col, DEFAULT_COLOR);
        if (sokoban.highscores[i].num_ticks != DEFAULT_SCORE) {
            put_time_at_loc(sokoban.highscores[i].num_ticks,
                            curr_draw_row, time_draw_col);
        }
        curr_draw_row += ELEMENT_ROW_SPACING;
    }
}

void sokoban_initialize_and_run()
{
    score_t default_highscore = { DEFAULT_SCORE, DEFAULT_SCORE };

    int i;
    /* initialize default highscores */
    for (i = 0; i < NUM_HIGHSCORES; i++) {
        sokoban.highscores[i] = default_highscore;
    }
    sokoban.state = INTRODUCTION;
    sokoban.previous_state = INTRODUCTION;

    display_introduction();

    /* poll for and handle inputs as we receive them */
    int ch;
    while (1) {
        do {
            ch = readchar();
        } while (ch == -1);
        handle_input(ch);
    }
}
