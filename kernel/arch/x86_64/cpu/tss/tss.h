#ifndef TSS_H
#define TSS_H

#include <stdint.h>

struct tss_t {
    uint32_t reserved0;

    uint64_t rsp0; // Ring 0 -- when interrupt rsp = tss.rsp0
    uint64_t rsp1; // Ring 1
    uint64_t rsp2; // Ring 2

    uint64_t reserved1;

    uint64_t ist1; // special interrupt stack table
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;

    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
};


void init_tss();

static void load_tss(uint8_t segment) {
    __asm__("ltr %%ax" : : "a"(segment));
}

#endif // TSS_H