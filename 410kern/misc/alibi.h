/** @file alibi.h
 *  @brief Level-description interface for "The Alibi"
 *
 *  @author Amolak Nagi (amolakn)
 */

#ifndef _ALIBI_H_
#define _ALIBI_H_

#define MAX_LEVELS 6

// Struct containing information about one level in The Alibi.

// Square colors represented in board strings
#define SQUARE_BLUE    ('B')
#define SQUARE_MAGENTA ('M')
#define SQUARE_NONE    (' ')

// Square edge and vertex representations in string form.
// You certainly shouldn't visualize them in your console
// like this
#define EDGE_HORIZONT  ('-')
#define EDGE_VERTICAL  ('|')
#define EDGE_NONE      ('_')

#define VERTEX_START   ('s')
#define VERTEX_REQUIRE ('*')
#define VERTEX_END     ('e')
#define VERTEX_NORMAL  ('+')

typedef struct alibilevel {
  int width;
  int height;
  char *board;
} alibilevel_t;

extern alibilevel_t *alibi_levels[MAX_LEVELS];
extern int alibi_nlevels;

#endif /* _ALIBI_H_ */
