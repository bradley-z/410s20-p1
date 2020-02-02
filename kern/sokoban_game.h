/** @file sokoban_game.h
 *  @brief game interface
 *
 *  This contains all the types and enums used in implementation of the sokoban
 *  game as well as the function declarations for the functions to be called
 *  externally.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug No known bugs.
 */
#ifndef __SOKOBAN_GAME_H_
#define __SOKOBAN_GAME_H_

#include <sokoban.h>
#include <stdbool.h>    /* bool */

#define NUM_HIGHSCORES 3

/**
 *  from what side of the image we are trying to align; further explained in
 *  sokoban_game.c
 */
typedef enum {
    TOP_SIDE,
    BOTTOM_SIDE,
    LEFT_SIDE,
    RIGHT_SIDE,
    CENTER,
} alignment_t;

/* movement direction */
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
} dir_t;

/* utilized to keep track of the state of an actively running game */
typedef enum {
    RUNNING,            /* game is actively running */
    PAUSED,             /* game is paused or in instructions */
    IN_LEVEL_SUMMARY,   /* level just completed and waiting for any keypress */
} game_state_t;

/* utilized to keep track of the state of the program */
typedef enum {
    INTRODUCTION,       /* intro screen */
    INSTRUCTIONS,       /* instructions screen */
    GAME_RUNNING,       /* active game session */
} sokoban_state_t;

/* utilized to keep track of the state of a running game */
typedef struct {
    sokolevel_t *level;         /* level metadata */
    int level_number;           /* number of level (not zero indexed) */
    unsigned int total_ticks;   /* total number of ticks across all levels */
    unsigned int level_ticks;   /* number of ticks for just current level */
    unsigned int total_moves;   /* total number of moves across all levels */
    unsigned int level_moves;   /* number of moves for just current level */
    bool on_goal;               /* if we are currently standing on a goal */

    int curr_row;               /* what row our player is at */
    int curr_col;               /* what column our player is at */
    int boxes_left;             /* boxes we still have to put on a goal */

    game_state_t game_state;    /* state of actively running game */
} game_t;

/* scoring is defined only by the number of moves first, then time second */
typedef struct {
    unsigned int num_moves;
    unsigned int num_ticks;
} score_t;

/* metadata of the program */
typedef struct {
    score_t highscores[NUM_HIGHSCORES]; /* just keep highscores in an array */
    sokoban_state_t state;              /* state of the program */
    sokoban_state_t previous_state;     /* utilized if state is INSTRUCTIONS */
} sokoban_t;

/** @brief contains the actual game logic to be done upon timer interrupt
 *
 *  We don't actually care for numTicks so we have no parameters. The important
 *  thing is that this function is called every ~10ms. If we're not in an active
 *  game or the game is paused/in level summary, then we do nothing. Otherwise,
 *  increment the number of ticks in the current level of the game. I decided
 *  to display time up to 0.1 second granularity, so whenever the level ticks is
 *  divisble by 10, we update the displayed time.
 *
 *  @return Void.
 */
void sokoban_tickback(void);
/** @brief initializes highscores and initial states then polls for inputs
 *
 *  Initialize all highscores to UINT32_MAX, set states to INTRODUCTION,
 *  displays the introduction screen, then loops indefinitely to poll for and
 *  handle input keypress events.
 *
 *  @return Void.
 */
void sokoban_initialize_and_run(void);

#endif /* __SOKOBAN_GAME_H_ */
