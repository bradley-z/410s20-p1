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
//     soko_level_t *level;
//     bool running;
// } board_state_t;

typedef struct {
    unsigned int num_moves;
    unsigned int time_seconds;
} score_t;

typedef struct {
    score_t hiscores[3];
} game_t;

void draw_image(int start_row, int start_col,
                int height, int width,
                int color, const char *image);
void display_introduction(void);
void sokoban_initialize_and_run(void);

#endif /* __SOKOBAN_GAME_H_ */
