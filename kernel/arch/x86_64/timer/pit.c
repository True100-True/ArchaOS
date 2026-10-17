#include "pit.h"

#define PIT_CHANNEL_0 0x40
#define PIT_MODE_REQ  0x43

// IRQ, where to pulse, count how long to pulse
void pit_count(uint8_t irq, uint64_t count) {
    asm("cli");
    outb(PIT_CHANNEL_0, count & 0xFF);
    outb(PIT_CHANNEL_0, (count & 0xFF00) >> 8);
    return;
}

