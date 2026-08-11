#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_FREQUENCY 100

void timer_initialize(void);
void timer_tick(void);

uint32_t timer_get_ticks(void);

uint32_t timer_get_uptime_seconds(void);

#endif