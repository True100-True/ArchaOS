#ifndef PMM_H
#define PMM_H

// Oh yes so sexy
#include <stdint.h>
#include "../../../lib/stdbool.h"
#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#define PAGE_SIZE 0x1000
#define SIZEOF_BITMAP (1024*1024)
#define MAX_PAGES (SIZEOF_BITMAP * 8)

void init_pmm(
    uint64_t mem_map,
    uint64_t mem_map_size,
    uint64_t descriptor_size
);

uint64_t palloc_page(void);
void pfree_page(uint64_t address);
void reserve_region(
    uint64_t base,
    uint64_t length
);

uint64_t get_free_pages();
uint64_t get_total_pages();


#endif