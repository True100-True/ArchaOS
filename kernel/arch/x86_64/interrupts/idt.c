#include "idt.h"

struct idt_entry idt[256];
bool idt_changed = false;

void idt_set_gate(uint8_t vector, void (*handler)(void)) {
    uint64_t addr = (uintptr_t)handler;
    idt[vector].offset_low   = addr & 0xFFFF;
    idt[vector].selector     = KERNEL_CS; // Kernel code segment
    idt[vector].ist          = 0;
    idt[vector].type_attr    = 0x8E;   // interrupt gate, present
    idt[vector].offset_mid   =(addr >> 16) & 0xFFFF;
    idt[vector].offset_high  =(addr >> 32);
    idt[vector].zero         = 0;
}

void idt_set_custom_gate(uint8_t vector, void (*handler)(void)) {
    uint64_t addr = (uintptr_t)handler;
    idt[vector].offset_low   = addr & 0xFFFF;
    idt[vector].selector     = KERNEL_CS; // Kernel code segment
    idt[vector].ist          = 0;
    idt[vector].type_attr    = 0x8E;   // interrupt gate, present
    idt[vector].offset_mid   =(addr >> 16) & 0xFFFF;
    idt[vector].offset_high  =(addr >> 32);
    idt[vector].zero         = 0;   
}

// IDT vector manipulation (just a wrapper but with some modifics)
void set_idt_vector(uint8_t vector, void (*handler)(void)) {
    if (idt[vector].offset_low != 0) { return; }
    if (idt_changed == false)
        idt_changed = true;
    idt_set_gate(vector, handler); // Will set it for a flush
}

// This is a flush of IDT vectors only IF idt table changed
void flush_idt_vector() {
    if (idt_changed)
        load_idt(sizeof(idt)-1, (uint64_t)&idt);
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