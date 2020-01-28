#include <timer.h>
#include <asm.h>
#include <timer_defines.h>

void timer_initialize(timer_t *timer, void (*tickback)(unsigned int)) {
    timer->numTicks = 0;
    timer->tickback = tickback;

    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    uint8_t period_lsb = (uint8_t)(CYCLES_10_MS | 0xFF);
    uint8_t period_msb = (uint8_t)(CYCLES_10_MS >> 8 | 0xFF);
    outb(TIMER_PERIOD_IO_PORT, period_lsb);
    outb(TIMER_PERIOD_IO_PORT, period_msb);
}

void timer_tick(timer_t *timer) {
    timer->numTicks++;
    if (timer->tickback) {
        timer->tickback(timer->numTicks);
    }
}