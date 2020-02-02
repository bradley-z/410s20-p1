/** @file timer.h
 *  @brief timer interface
 *
 *  Interface for a timer, which keeps track of the number of timer interrupts
 *  received and a callback function that is called with the number of ticks.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug The internal rate of the timer as defined by timer_defines.h is
 *       1193182 Hz (cycles per second). Since we want our timer to fire every
 *       10 milliseconds, we need to divide that internal rate by 100 to get the
 *       number of cycles per 10 milliseconds. The actual result is not a whole
 *       number, so the timer is ever so slightly (0.0015%) slow.
 */
#ifndef __TIMER_H_
#define __TIMER_H_

#include <timer_defines.h>  /* TIMER_RATE */

/* 100 ten ms intervals in 1 sec, so do TIMER_RATE / 100 to get cycles / 10ms */
#define CYCLES_10_MS (TIMER_RATE / 100)

typedef struct {
    unsigned int numTicks;
    void (*tickback)(unsigned int);
} timer_t;

/* global timer struct that keeps track of ticks and callback */
timer_t timer;

/** @brief sets tickback callback function for a timer
 *
 *  Due to design choice for not initializing our timer completely in
 *  handler_install(), this function provides flexibilty for initializing the
 *  timer in, for example, game.c, and then having the tickback function be
 *  supplied later, such as to the handler_install() function
 *
 *  @param timer pointer to timer to initialize
 *  @param tickback pointer to callback function to set
 *  @return Void.
 */
void timer_set_tickback(timer_t *timer, void (*tickback)(unsigned int));
/** @brief initializes the timer
 *
 *  Sets numTicks to 0, sets callback function, then sends data to the timer io
 *  port to initialize the internal timer.
 *
 *  @param timer pointer to timer to initialize
 *  @param tickback pointer to callback function to set
 *  @return Void.
 */
void timer_initialize(timer_t *timer, void (*tickback)(unsigned int));
/** @brief called upon timer firing
 *
 *  Increments numTicks and calls the tickback callback function. This is called
 *  by the timer handler.
 *
 *  @return Void.
 */
void timer_tick(timer_t *timer);

#endif /* __TIMER_H_ */
