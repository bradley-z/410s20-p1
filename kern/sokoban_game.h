#ifndef __SOKOBAN_GAME_H_
#define __SOKOBAN_GAME_H_

#include <sokoban.h>
#include <stdbool.h>

typedef enum {
    TOP_SIDE,
    BOTTOM_SIDE,
    LEFT_SIDE,
    RIGHT_SIDE,
    CENTER,
} alignment_t;

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

void sokoban_tickback(void);
void sokoban_initialize_and_run(void);

#endif /* __SOKOBAN_GAME_H_ */
