#include "idt.h"

void idt_set_gate(struct idt_entry idt[256], uint8_t vector, void (*handler)(void)) {
    uint64_t addr = (uint64_t)handler;
    idt[vector].offset_low =
        addr & 0xFFFF;
    idt[vector].selector =
        0x08;   // your kernel code segment
    idt[vector].ist = 0;
    idt[vector].type_attr =
        0x8E;   // interrupt gate, present
    idt[vector].offset_mid =
        (addr >> 16) & 0xFFFF;
    idt[vector].offset_high =
        (addr >> 32);
    idt[vector].zero = 0;
}

/*
void init_idt() {
    // Faults (cpu raised)
    idt_set_gate(0, divide_handler);
    idt_set_gate(6, invalid_opcode_handler);
    idt_set_gate(13, general_protection_handler);
    idt_set_gate(14, page_fault_handler);
    idt_set_gate(8, double_fault_handler);

    // IRQ's 32 - 44
    idt_set_gate(32, tick_interrupt);

    // 44 - 255 Other


    load_idt(sizeof(idt)-1, (uint64_t)&idt);
}
*/