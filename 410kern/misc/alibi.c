/** @file alibi.c
 *  @brief Board descriptions for "The Alibi".
 *
 *  @author Amolak Nagi (amolakn)
 */

#include "alibi.h"

alibilevel_t level1 = {3, 3, "\
+-+-+-e\
|B|B|B|\
+-+-+_+\
|B|M|B|\
+-+-+-+\
|M|M|M|\
s-+-+-*"};

// https://windmill.thefifthmatt.com/2c4hb2g
alibilevel_t level2 = {3, 3, "\
+-+-+-e\
|M|B| |\
+-+-+-+\
|B| |M|\
+-+-+-+\
| |M| |\
s-+-+-*"};

// Derived from:
// https://windmill.thefifthmatt.com/7t505pg
alibilevel_t level3 = {4, 3, "\
+-+-*-+-+\
|B| | | |\
+-+-+-+-+\
| |B|B|B|\
s-+-+-+-e\
| |M|M| |\
+-+-+-+-+"};

alibilevel_t *alibi_levels[] = {&level1, &level2, &level3, 0, 0, 0};
int alibi_nlevels = 3;
