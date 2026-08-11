#include "timer.h"
#include "io.h"

static volatile uint32_t timer_ticks = 0;

void timer_initialize(void)
{
    uint32_t divisor;

    /*
     * PIT base frequency:
     *
     * 1,193,182 Hz
     */
    divisor = 1193182 / TIMER_FREQUENCY;

    /*
     * Command register.
     *
     * 0x36:
     * Channel 0
     * Access mode: low byte / high byte
     * Mode 3: square wave
     * Binary mode
     */
    outb(0x43, 0x36);

    /*
     * Send divisor low byte.
     */
    outb(0x40, (uint8_t)(divisor & 0xFF));

    /*
     * Send divisor high byte.
     */
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void timer_tick(void)
{
    timer_ticks++;
}

uint32_t timer_get_ticks(void)
{
    return timer_ticks;
}

uint32_t timer_get_uptime_seconds(void)
{
    return timer_ticks / TIMER_FREQUENCY;
}