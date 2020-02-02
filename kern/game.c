/** @file game.c
 *  @brief A kernel with timer, keyboard, console support
 *
 *  This file contains the kernel's main() function.
 *
 *  It sets up the drivers and starts the game.
 *
 *  @author Michael Berman (mberman)
 *  @bug No known bugs.
 */

#include <p1kern.h>

/* Think about where this declaration
 * should be... probably not here!
 */
void tick(unsigned int numTicks);

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */
#include <malloc.h>

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* memory includes. */
#include <lmm.h>                    /* lmm_remove_free() */

/* x86 specific includes */
#include <x86/seg.h>                /* install_user_segs() */
#include <x86/interrupt_defines.h>  /* interrupt_setup() */
#include <x86/asm.h>                /* enable_interrupts() */

#include <string.h>

#include <sokoban.h>
#include <sokoban_game.h>

#include <timer.h>
#include <kb_buffer.h>

/* timer defined in timer.h */
extern timer_t timer;
/* keyboard buffer defined in kb.c */
extern kb_buf_t kb_buffer;

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.  It simply sets up the
 *  drivers and passes control off to game_run().
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    /*
     * Initialize device-driver library.
     */
    timer_initialize(&timer, NULL);
    kb_buf_initialize(&kb_buffer);

    handler_install(tick);

    enable_interrupts();

    clear_console();

    hide_cursor();

    set_term_color(FGND_WHITE | BGND_BLACK);

    sokoban_initialize_and_run();

    return 0;
}

/** @brief Tick function, to be called by the timer interrupt handler
 * 
 *  In a real game, this function would perform processing which
 *  should be invoked by timer interrupts.
 *
 **/
void tick(unsigned int numTicks)
{
    sokoban_tickback();
}
