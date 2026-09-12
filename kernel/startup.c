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
#include "graphics/graphics.h"
#include "config.h"
#include "kcode/arch.h"
#include "errs/pnc.h"
#include "lib/stdio.h"

/*
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
*/

void kernel_main(BootInfo *boot) {
    /*
    serial_putchar('K');
    serial_putchar('R');
    serial_putchar('N');
    serial_putchar('L');
    */

    const char *kernel_version = "0.1.1"; 

    serial_init();

    serial_print("\n\nKernel booted...\n\n");
    serial_print("Version ");
    serial_print(kernel_version);
    serial_print("\n");
    serial_print("[+] Checking post-boot information\n");
    serial_print("boot ptr: ");
    serial_print_hex((uint64_t)boot);
    serial_print("magic: ");
    serial_print_hex(boot->magic);

    if (boot == 0 || boot->magic != BOOTINFOMAGIC) {
        serial_print("\t[!] Post-boot information table -- missing magic...\n");
        return; // I want to return to bootloader but bootloaders are retarted when somethin like this happens
    }
    if (boot->framebuffer_base == 0) {
        serial_print("\t[!] Frame buffer base addr is missing...\n");
        return;
    }
    if (boot->width == 0 || boot->height == 0) {
        serial_print("\t[!] Sizes (of frame buffer) are missing...\n");
        return;
    }

    // MOVE THIS TO GRAPHICS.h !!!!!!!!!!

    serial_print("Switching to graphical output..\n");
    framebuffer_t framebuffer_fb = {0};
    framebuffer_fb.address = (uint32_t *)boot->framebuffer_base;
    framebuffer_fb.height = boot->height;
    framebuffer_fb.width = boot->width;
    framebuffer_fb.pitch = boot->pixels_per_scanline;

    serial_print("Resolution:\nWidth: ");
    serial_print_hex(framebuffer_fb.width);
    serial_print("Height: ");
    serial_print_hex(framebuffer_fb.height);
    serial_print("Pitch: ");
    serial_print_hex(framebuffer_fb.pitch);
    serial_print("\n");

    // MUST be in this anarchy, First graphics, then print
    init_graphics(&framebuffer_fb);
    print_init();
    serial_print("Loaded framebuffer\n");

    // TILL THIS !!!!!!!!!!

    print("ArchaOS %s \n", kernel_version);
    print("\n");

    // Arch depented code following !!!'
    // Both of these are located in "kcode/arch.h" that chooses arch and picks the right folder
    init_cpu();        // The stuff like GDT, IDT, TSS, etc so CPU
    init_memory(boot); // Similar to above, but PMM, VMM, HMM, etc. (because of cr3 and MMU)

    print("\n\n");

    print("Total memory: %n MB\n", total_memory / (1024*1024));
    print("Total usable memory: %n MB\n\n", total_usable_memory / (1024*1024));

    print("Kernel physical addr: %lx \n", boot->kernel_physical_address);
    print("Kernel virtual  addr: %lx \n\n", boot->kernel_virtual_address);

    while (1)
        asm volatile("hlt");
}