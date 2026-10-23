#ifndef TIME_H
#define TIME_H

// Experimental functions

#include "math.h"
#include "../kcode/arch.h"

// Sleep kernel for x ms
static void kernel_sleep_ms(uint64_t time_ms) {
    // assuming current timer IS started and that it's maximal resolution is 1MS
    uint64_t start_time = get_milliseconds(); // Get curent tick time
    while (abs(start_time - get_milliseconds()) <= time_ms)
        asm("hlt");
}

#endif //TIME_H