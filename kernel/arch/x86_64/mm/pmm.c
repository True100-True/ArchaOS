// This shit is PMM.c (physical memory manager)
// DO NOT try to fuck this up or wont boot up properly
// atleast to the point of PMM test so yeah

#include "pmm.h"

uint64_t sizeof_bitmap;
uint64_t max_pages;

uint8_t *bitmap;
uint8_t bitmap_addr;
bool bitmap_init = false;

uint64_t free_pages = 0;
uint64_t total_pages = 0;
uint64_t total_ram_pages = 0;

uint64_t total_memory = 0;
uint64_t total_usable_memory = 0;

uint64_t max_physical_address = 0;

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


static bool page_used(uint64_t page) {
    if(page >= max_pages)
        return true;
    return bitmap[page / 8] &
           (1 << (page % 8));
}
static void mark_used(uint64_t page) {
    if(page >= max_pages)
        return;
    if(!page_used(page)) {
        bitmap[page / 8] |=
            (1 << (page % 8));
        free_pages--;
    }
}
static void mark_free(uint64_t page) {
    if(page >= max_pages)
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

    for (uint64_t byte = 0; byte < sizeof_bitmap; byte++) {
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
uint64_t palloc(void) {
    if (!bitmap_init)
        return 0;

    for (uint64_t byte = 0;
         byte < sizeof_bitmap;
         byte++) {
        if (bitmap[byte] == 0xFF)
            continue;

        for (uint64_t bit = 0; bit < 8; bit++) {
            uint8_t mask =
                (uint8_t)(1U << bit);

            if (!(bitmap[byte] & mask)) {
                uint64_t page =
                    byte * 8 + bit;

                if (page >= max_pages)
                    return 0;

                mark_used(page);

                return page * PAGE_SIZE;
            }
        }
    }

    return 0;
}

/* void reserve_region(uint64_t base, uint64_t length) {
    regions[region_count].base = base;
    regions[region_count].length = length;
    //regions[region_count].type = type;

    region_count++;
}
*/

void pfree(uint64_t address) {
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
    free_pages = 0;
    total_pages = 0;

    max_physical_address = 0;

    uint64_t entries = mem_map_size / descriptor_size;
    for(uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*) (
                (uint8_t*)mem_map + i * descriptor_size );
        if (desc->Type == EfiConventionalMemory) {
            uint64_t end = desc->PhysicalStart + desc->NumberOfPages * PAGE_SIZE;
            if (end > max_physical_address)
                max_physical_address = end;
            total_pages += desc->NumberOfPages;
            print(
                "Region pages: %n, total: %n\n",
                desc->NumberOfPages,
                total_pages
            );
        
            switch (desc->Type) {
                case EfiConventionalMemory:
                case EfiLoaderCode:
                case EfiLoaderData:
                case EfiBootServicesCode:
                case EfiBootServicesData:
                case EfiRuntimeServicesCode:
                case EfiRuntimeServicesData:
                case EfiACPIReclaimMemory:
                case EfiACPIMemoryNVS:
                    total_ram_pages += desc->NumberOfPages;
                    break;
            }
        }
    }

    max_pages = max_physical_address / PAGE_SIZE;
    sizeof_bitmap = (max_pages + 7) / 8;

    //memset(bitmap, 0xFF, sizeof_bitmap);

    for (uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*) (
                (uint8_t*)mem_map + i * descriptor_size );
        if (desc->Type != EfiConventionalMemory)
            continue;
        // hunt for large enough memory chunk
        uint64_t mem_chunk_size = desc->NumberOfPages * PAGE_SIZE;
        if (mem_chunk_size >= sizeof_bitmap) {
            bitmap_addr = desc->PhysicalStart;
        }   
    } 

    bitmap = (void *)bitmap_addr;
    memset((void *)bitmap, 0xFF, sizeof_bitmap);

    for(uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*) (
                (uint8_t*)mem_map + i * descriptor_size );

        if(desc->Type == EfiConventionalMemory) {
            uint64_t start = desc->PhysicalStart / PAGE_SIZE;
            for(uint64_t p = 0; p < desc->NumberOfPages; p++) {
                mark_free(start + p);
            }
        }
    }
    

    total_usable_memory = total_pages * PAGE_SIZE;
    total_memory = total_ram_pages * PAGE_SIZE;

    print("Total memory: %n MB\n", total_memory / (1024*1024));
    print("Total usable memory: %n MB\n", total_usable_memory / (1024*1024));

    uint64_t bitmap_start_page = bitmap_addr / PAGE_SIZE;
    uint64_t bitmap_pages = (sizeof_bitmap + PAGE_SIZE - 1) / PAGE_SIZE;

    mark_used(0);
    for (uint64_t i = 0;i < bitmap_pages;i++) {
        mark_used(bitmap_start_page+i);
    }
    

    bitmap_init = true;
}
