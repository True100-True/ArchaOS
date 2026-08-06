/*
typedef struct {
    uint64_t magic;
    void (*entry)(BootInfo*);
} KernelHeader;

__attribute__((section(".header")))
KernelHeader header = {
    0xCAFEBABE,
    kernel_main
};
*/
#include "../common/fromboot.h"
#include "../common/fonts/cozette.h"

#include "kcode/gdt.h"
#include "kcode/idt.h"
#include "kcode/io.h"

#include "mem/pmm.h"

#include "lib/stdio.h"

void kernel_crash(void) {
    asm volatile("ud2");
    while (1)
        asm volatile("hlt");
}

void kernel_main(BootInfo* boot) {
    /*
    serial_putchar('K');
    serial_putchar('R');
    serial_putchar('N');
    serial_putchar('L');
    */

    serial_init();
    
    serial_print("Kernel booted...\n\n");
    serial_print("[+] Checking post-boot information\n");
    serial_print("boot ptr: ");
    serial_print_hex((uint64_t)boot);
    serial_print("magic: ");
    serial_print_hex(boot->magic);

    if (boot == 0 || boot->magic != BOOTINFOMAGIC) {
        serial_print("\t[!] Post-boot information table -- missing magic...\n");
        return;
    }
    if (boot->framebuffer_base == 0) {
        serial_print("\t[!] Frame buffer base addr is missing...\n");
    }
    if (boot->width == 0 || boot->height == 0) {
        serial_print("\t[!] Sizes (of frame buffer) are missing...\n");
    }

    /*
    uint32_t* fb = (uint32_t*)boot->framebuffer_base;

    for (uint64_t y = 0; y < boot->height; y++) {
        for (uint64_t x = 0; x < boot->width; x++) {
            fb[y * boot->pixels_per_scanline + x] = 0x000000;
        }
    }
    */

    serial_print("[+] Loading GDT..\n");
    init_gdt();
    serial_print("[~] GDT loaded...\n");

    serial_print("[+] Loading IDT..\n");
    init_idt();
    serial_print("[~] IDT loaded...\n");

    serial_print("[+] Loading PPM..\n");
    init_pmm(boot->memory_map, boot->memory_map_size, boot->memory_descriptor_size);
    serial_print("[~] Loaded PPM..\n");


    
    
    while (1)
        asm volatile("hlt");
}