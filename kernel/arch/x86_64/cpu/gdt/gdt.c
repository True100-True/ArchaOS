#include "gdt.h"

#include "gdt.h"
#include <stdint.h>
struct gdt_entry {
    uint64_t value;
};

struct gdt_entry gdt[7];

uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
    uint64_t descriptor;

    descriptor  = limit & 0x000F0000;
    descriptor |= (flag << 8) & 0x00F0FF00;
    descriptor |= (base >> 16) & 0x000000FF;
    descriptor |= base & 0xFF000000;

    descriptor <<= 32;

    descriptor |= base << 16;
    descriptor |= limit & 0xFFFF;

    return descriptor;
}

static void set_tss_descriptor(uintptr_t base, uint32_t limit)
{
    uint64_t low = 0;
    uint64_t high = 0;

    low |= (uint64_t)(limit & 0xFFFF);
    low |= (uint64_t)(base & 0xFFFF) << 16;
    low |= (uint64_t)((base >> 16) & 0xFF) << 32;
    low |= (uint64_t)0x89 << 40;  // Present, DPL 0, available 64-bit TSS
    low |= (uint64_t)((limit >> 16) & 0x0F) << 48;
    low |= (uint64_t)((base >> 24) & 0xFF) << 56;

    high = (uint64_t)(base >> 32); // Upper 32 bits of the base

    gdt[TSS_GDT_INDEX].value     = low;
    gdt[TSS_GDT_INDEX + 1].value = high;
}

void init_gdt() {
    gdt[0].value = 0;

    gdt[1].value = create_descriptor(
        0,
        0xFFFFF,
        GDT_CODE_PL0
    );

    gdt[2].value = create_descriptor(
        0,
        0xFFFFF,
        GDT_DATA_PL0
    );

    gdt[3].value = create_descriptor(
        0,
        0xFFFFF,
        GDT_CODE_PL3
    );

    gdt[4].value = create_descriptor(
        0,
        0xFFFFF,
        GDT_DATA_PL3
    );

    // TSS
    init_tss();
    
    set_tss_descriptor(
        0, 
        0xFFFFF
    );

    load_gdt(
        sizeof(gdt)-1,
        (uint64_t)&gdt
    );

    // HERE IT CAUSES ERRORS TODO
    load_tss(TSS_SELECTOR);
}