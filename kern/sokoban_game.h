#ifndef __SOKOBAN_GAME_H_
#define __SOKOBAN_GAME_H_

#include <sokoban.h>
#include <stdbool.h>

// typedef enum {
//     RUNNING,
//     PAUSED,
// } state_t;

// typedef struct {
//     int number;
//     sokolevel_t *level;
//     bool running;
// } board_state_t;

typedef enum {
    INTRODUCTION,
    INSTRUCTIONS,
    LEVEL_RUNNING,
} game_state_t;

typedef struct {
    unsigned int num_moves;
    unsigned int time_seconds;
} score_t;

typedef struct {
    score_t hiscores[3];
    game_state_t game_state;
    game_state_t previous_state;
} game_t;

void handle_input(char ch);
char poll_for_input();
void draw_image(int start_row, int start_col,
                int height, int width,
                int color, const char *image);
void display_instructions(void);
void display_introduction(void);
void sokoban_initialize_and_run(void);

#endif /* __SOKOBAN_GAME_H_ */
