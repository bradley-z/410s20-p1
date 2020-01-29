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

// typedef struct {
//     int number;
//     sokolevel_t *level;
//     bool running;
// } board_state_t;

typedef struct {
    sokolevel_t *level;
    int level_number;
    int cur_row;
    int cur_col;
    unsigned int num_ticks;
    unsigned int total_moves;
    unsigned int level_moves;
    game_state_t game_state;
} game_t;

typedef enum {
    INTRODUCTION,
    INSTRUCTIONS,
    LEVEL_RUNNING,
} sokoban_state_t;

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
int16_t draw_sokoban_level(sokolevel_t *level);
void handle_input(char ch);
char poll_for_input();
void draw_image(int start_row, int start_col,
                int height, int width,
                int color, const char *image);
void start_game(void);
void display_instructions(void);
void display_introduction(void);
void sokoban_initialize_and_run(void);

#endif /* __SOKOBAN_GAME_H_ */
