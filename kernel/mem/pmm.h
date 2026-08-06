#ifndef PMM_H
#define PMM_H

#include "../lib/stdint.h"
#include "../lib/stdbool.h"
#include "../lib/stdio.h"
#include "../lib/string.h"

void init_pmm(
    uint64_t mem_map,
    uint64_t mem_map_size,
    uint64_t descriptor_size
);

void* alloc_page(bool erase);
void free_page(void* address);
void reserve_region(
    uint64_t base,
    uint64_t length
);

uint64_t get_free_pages();
uint64_t get_total_pages();


#endif