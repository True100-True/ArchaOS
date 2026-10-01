#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "pic.h"

extern void tick_interrupt(void);
volatile void clock_interrupt(void);
uint64_t get_ticks();
uint64_t get_milliseconds(void);

#endif // CLOCK_H