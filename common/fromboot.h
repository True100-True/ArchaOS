#ifndef AFTERBOOT_H
#define AFTERBOOT_H

#include <stdint.h>

#define BOOTINFOMAGIC 0xCAFEBABEDEADBEEF

typedef struct {
    uint64_t magic;

    uint64_t framebuffer_base;
    uint64_t framebuffer_size;

    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;

    uint64_t memory_map;
    uint64_t memory_map_size;
    uint64_t memory_descriptor_size;

    uint64_t kernel_physical_address;
    uint64_t kernel_physical_address_start;
    uint64_t kernel_physical_address_end;

    uint64_t kernel_virtual_address;
    
    uint64_t kernel_size;
} BootInfo;

#endif