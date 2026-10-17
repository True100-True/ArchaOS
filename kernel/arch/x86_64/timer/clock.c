#include "clock.h"

uint64_t total_ticks = 0;

void clock_interrupt(void) {
    ++total_ticks;
    apic_eoi();
}

uint64_t get_ticks() {
    return total_ticks;
}

uint64_t get_milliseconds(void) {
    return (get_ticks() * 1000) / 100;
}

void timer_reset(void) {
    total_ticks = 0;
}

void start_clock() {
    apic_timer_start();
}
void stop_clock() {
    apic_timer_stop();
}