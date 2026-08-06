#include "idt.h"


struct idt_entry idt[256];


void idt_set_gate(uint8_t vector, void (*handler)(void))
{
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

extern void divide_handler();
extern void invalid_opcode_handler();
extern void general_protection_handler();
extern void page_fault_handler();

extern void load_idt(uint16_t size, uint64_t base);

void init_idt()
{
    idt_set_gate(0, divide_handler);
    idt_set_gate(6, invalid_opcode_handler);
    idt_set_gate(13, general_protection_handler);
    idt_set_gate(14, page_fault_handler);

    load_idt(sizeof(idt)-1, (uint64_t)&idt);
}