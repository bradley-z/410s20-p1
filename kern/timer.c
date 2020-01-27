#include <timer.h>

void timer_initialize(timer_t *timer, void (*tickback)(unsigned int)) {
    timer->numTicks = 0;
    timer->tickback = tickback;
}

void timer_tick(timer_t *timer) {
    timer->numTicks++;
    if (timer->tickback) {
        timer->tickback(timer->numTicks);
    }
}