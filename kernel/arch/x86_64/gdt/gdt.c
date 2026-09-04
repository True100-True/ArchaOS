#include "gdt.h"

#include "gdt.h"
#include <stdint.h>

struct gdt_entry {
    uint64_t value;
};

struct gdt_entry gdt[5];

uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
{
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

void init_gdt()
{
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

    load_gdt(
        sizeof(gdt)-1,
        (uint64_t)&gdt
    );
}