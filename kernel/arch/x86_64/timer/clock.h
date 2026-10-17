#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "../interrupts/apic.h"

extern void tick_interrupt(void);
volatile void clock_interrupt(void);
uint64_t get_ticks();
uint64_t get_milliseconds(void);

void timer_reset(void);
void start_clock();
void stop_clock();

#endif // CLOCK_H