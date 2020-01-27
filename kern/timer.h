#ifndef __TIMER_H
#define __TIMER_H

typedef struct {
    unsigned int numTicks;
    void (*tickback)(unsigned int);
} timer_t;

void timer_initialize(timer_t *timer, void (*tickback)(unsigned int));
void timer_tick(timer_t *timer);

#endif /* __TIMER_H */