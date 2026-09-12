#ifndef PMM_H
#define PMM_H

// Oh yes so sexy
#include <stdint.h>
#include "../../../lib/stdbool.h"
#include "../../../lib/stdio.h"
#include "../../../lib/string.h"

#define PAGE_SIZE   0x1000
#define HUGE_PAGE   0x200000ULL

//#define SIZEOF_BITMAP (1024*1024)
//#define MAX_PAGES (SIZEOF_BITMAP * 8)

extern uint64_t sizeof_bitmap;
extern uint64_t max_pages;

extern uint8_t *bitmap;
extern uint8_t bitmap_addr;
extern bool bitmap_init;

extern uint64_t free_pages;
extern uint64_t total_pages;

extern uint64_t total_usable_memory;
extern uint64_t total_memory;

extern uint64_t max_physical_address;

void init_pmm(
    uint64_t mem_map,
    uint64_t mem_map_size,
    uint64_t descriptor_size
);

uint64_t palloc(void);
void pfree(uint64_t address);
void reserve_region(
    uint64_t base,
    uint64_t length
);

uint64_t get_free_pages();
uint64_t get_total_pages();


#endif