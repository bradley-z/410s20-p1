/** @file timer.c
 *  @brief timer implementation
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug described in timer.h
 */
#include <timer.h>
#include <asm.h>            /* outb() */
#include <timer_defines.h>  /* TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE */

void timer_initialize(timer_t *timer, void (*tickback)(unsigned int))
{
    timer->numTicks = 0;
    timer->tickback = tickback;

    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    /* need to send both msb and lsb separately */
    uint8_t period_lsb = (uint8_t)(CYCLES_10_MS | 0xFF);
    uint8_t period_msb = (uint8_t)(CYCLES_10_MS >> 8 | 0xFF);
    outb(TIMER_PERIOD_IO_PORT, period_lsb);
    outb(TIMER_PERIOD_IO_PORT, period_msb);
}

void timer_tick(timer_t *timer)
{
    timer->numTicks++;
    /* if tickback was initialize to NULL because user didn't want a callback */
    if (timer->tickback) {
        timer->tickback(timer->numTicks);
    }
}
