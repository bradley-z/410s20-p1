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
} game_state_t;

typedef enum {
    INTRODUCTION,
    INSTRUCTIONS,
    LEVEL_RUNNING,
} sokoban_state_t;

typedef struct {
    int16_t total_boxes;
    int8_t start_row;
    int8_t start_col;
} level_info_t;

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
    unsigned int time_seconds;
} score_t;

typedef struct {
    score_t hiscores[3];
    sokoban_state_t state;
    sokoban_state_t previous_state;
} sokoban_t;

void sokoban_tickback(unsigned int numTicks);
level_info_t draw_sokoban_level(sokolevel_t *level);
void print_current_game_moves(void);
void print_current_game_time(void);
void level_up();
void try_move(dir_t dir);
void handle_input(char ch);
void draw_image(int start_row, int start_col,
                int height, int width,
                int color, const char *image);
void quit_game(void);
void restart_level(void);
void pause_game(void);
void start_game(void);
void display_instructions(void);
void display_introduction(void);
void sokoban_initialize_and_run(void);

#endif /* __SOKOBAN_GAME_H_ */
