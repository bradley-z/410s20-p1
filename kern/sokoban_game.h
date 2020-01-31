#ifndef __SOKOBAN_GAME_H_
#define __SOKOBAN_GAME_H_

#include <sokoban.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
} dir_t;

typedef enum {
    RUNNING,
    PAUSED,
    IN_LEVEL_SUMMARY,
} game_state_t;

typedef enum {
    INTRODUCTION,
    INSTRUCTIONS,
    LEVEL_RUNNING,
} sokoban_state_t;

typedef struct {
    sokolevel_t *level;
    int level_number;
    unsigned int total_ticks;
    unsigned int level_ticks;
    unsigned int total_moves;
    unsigned int level_moves;
    bool on_goal;

    int curr_row;
    int curr_col;
    int boxes_left;

    game_state_t game_state;
} game_t;

typedef struct {
    unsigned int num_moves;
    unsigned int num_ticks;
} score_t;

typedef struct {
    score_t hiscores[3];
    sokoban_state_t state;
    sokoban_state_t previous_state;
} sokoban_t;

void sokoban_tickback(unsigned int numTicks);

void draw_image(const char *image, int start_row, int start_col,
                int height, int width, int color);
bool draw_sokoban_level(sokolevel_t *level, int *total_boxes,
                        int *start_row, int *start_col);
void put_time_at_loc(int ticks, int row, int col);
void print_current_game_moves(void);
void print_current_game_time(void);
void putstring(const char *str, int row, int col, int color);

bool valid_next_square(dir_t dir, int row, int col, int *new_row, int *new_col);
void try_move(char ch);
void handle_input(char ch);
void level_up(void);
void complete_game(void);
void complete_level(void);
void quit_game(void);
void pause_game(void);
void restart_current_level(void);
void start_sokoban_level(int level_number);
void start_game(void);
void display_instructions(void);
void display_introduction(void);
void sokoban_initialize_and_run(void);

#endif /* __SOKOBAN_GAME_H_ */
