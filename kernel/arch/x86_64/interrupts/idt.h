#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include "../timer/clock.h" // For tick_timer

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;

    uint8_t type_attr;

    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));


struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

extern struct idt_entry idt[256];

void idt_set_gate(uint8_t vector, void (*handler)(void));
extern void load_idt(uint16_t size, uint64_t base);

//void init_idt();

#endif