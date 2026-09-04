// This shit is PMM.c (physical memory manager)
// DO NOT try to fuck this up or wont boot up properly
// atleast to the point of PMM test so yeah

#include "pmm.h"

typedef struct memory_region {
    uint64_t    base;
    uint64_t    length;
    //uint32_t    type;
} memory_region;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory, // OUR data
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct {
   uint32_t    Type;
   uint64_t    PhysicalStart;
   uint64_t    VirtualStart;
   uint64_t    NumberOfPages;
   uint64_t    Attribute;
} EFI_MEMORY_DESCRIPTOR;

//#define MAX_REGIONS 6000
//memory_region regions[MAX_REGIONS];
//int region_count = 0;

uint8_t bitmap[SIZEOF_BITMAP];
bool bitmap_init = false;
static uint64_t free_pages = 0;
static uint64_t total_pages = 0;

static bool page_used(uint64_t page) {
    if(page >= MAX_PAGES)
        return true;
    return bitmap[page / 8] &
           (1 << (page % 8));
}
static void mark_used(uint64_t page) {
    if(page >= MAX_PAGES)
        return;
    if(!page_used(page)) {
        bitmap[page / 8] |=
            (1 << (page % 8));
        free_pages--;
    }
}
static void mark_free(uint64_t page) {
    if(page >= MAX_PAGES)
        return;
    if(page_used(page)) {
        bitmap[page / 8] &=
            ~(1 << (page % 8));
        free_pages++;
    }
}
void reserve_region(uint64_t base, uint64_t length) {
    uint64_t start =
        base / PAGE_SIZE;
    uint64_t pages =
        (length + PAGE_SIZE - 1)
        / PAGE_SIZE;

    for(uint64_t i = 0; i < pages; i++) {
        mark_used(start + i);
    }
}
/*
void* palloc_page(bool erase) {
    if (!bitmap_init) {
        return NULL;
    }

    for (uint64_t byte = 0; byte < SIZEOF_BITMAP; byte++) {
        if (bitmap[byte] != 0xFF) {
            for (int bit = 0; bit < 8; bit++) {
                uint8_t mask = 1 << bit;
                if (!(bitmap[byte] & mask)) {
                    uint64_t page =
                        byte * 8 + bit;
                    mark_used(page);
                    uint64_t address =
                        page * PAGE_SIZE;
                    if (erase)
                        memset((void*)address, 0, PAGE_SIZE);
                    return (void*)address;
                }
            }
        }
    }
    return NULL;
}
*/
uint64_t palloc_page(void) {
    if (!bitmap_init)
        return 0;

    for (uint64_t byte = 0;
         byte < SIZEOF_BITMAP;
         byte++) {
        if (bitmap[byte] == 0xFF)
            continue;

        for (uint64_t bit = 0; bit < 8; bit++) {
            uint8_t mask =
                (uint8_t)(1U << bit);

            if (!(bitmap[byte] & mask)) {
                uint64_t page =
                    byte * 8 + bit;

                if (page >= MAX_PAGES)
                    return 0;

                mark_used(page);

                return page * PAGE_SIZE;
            }
        }
    }

    mark_used(0);

    return 0;
}

/* void reserve_region(uint64_t base, uint64_t length) {
    regions[region_count].base = base;
    regions[region_count].length = length;
    //regions[region_count].type = type;

    region_count++;
}
*/

void pfree_page(uint64_t address) {
    uint64_t page = address / PAGE_SIZE;
    mark_free(page);
}

uint64_t get_free_pages() {
    return free_pages;
}
uint64_t get_total_pages() {
    return total_pages;
}

void init_pmm(uint64_t mem_map, uint64_t mem_map_size, uint64_t descriptor_size) {
    memset(
        bitmap,
        0xFF,
        sizeof(bitmap)
    );

    free_pages = 0;
    total_pages = 0;

    uint64_t entries = mem_map_size / descriptor_size;
    for(uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*) (
                (uint8_t*)mem_map + i * descriptor_size );

        if(desc->Type == EfiConventionalMemory) {
            uint64_t start = desc->PhysicalStart / PAGE_SIZE;
            uint64_t pages = desc->NumberOfPages;
            for(uint64_t p = 0; p < pages; p++) {
                mark_free(start + p);
            }
            total_pages += pages;
        }
    }

    mark_used(0);
    bitmap_init = true;
}
