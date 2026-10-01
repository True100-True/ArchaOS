#include "clock.h"

uint64_t ticks_from_boot = 0;

void clock_interrupt(void) {
    ++ticks_from_boot;
    pic_acknowledge(0); // Change to support APIC aswell
}

uint64_t get_ticks() {
    return ticks_from_boot;
}

uint64_t get_milliseconds(void) {
    return (get_ticks() * 1000) / 100;
}
