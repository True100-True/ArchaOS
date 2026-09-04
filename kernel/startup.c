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

const char *kernel_version = "0.1.1"; 

void kernel_main(BootInfo *boot) {
    /*
    serial_putchar('K');
    serial_putchar('R');
    serial_putchar('N');
    serial_putchar('L');
    */

    serial_init();

    serial_print("\n\nKernel booted...\n\n");
    serial_print("Version");
    serial_print(kernel_panic);
    serial_print("\n");
    serial_print("[+] Checking post-boot information\n");
    serial_print("boot ptr: ");
    serial_print_hex((uint64_t)boot);
    serial_print("magic: ");
    serial_print_hex(boot->magic);

    if (boot == 0 || boot->magic != BOOTINFOMAGIC)
    {
        serial_print("\t[!] Post-boot information table -- missing magic...\n");
        return; // I want to return to bootloader but bootloaders are retarted when somethin like this happens
    }
    if (boot->framebuffer_base == 0)
    {
        serial_print("\t[!] Frame buffer base addr is missing...\n");
    }
    if (boot->width == 0 || boot->height == 0)
    {
        serial_print("\t[!] Sizes (of frame buffer) are missing...\n");
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
    serial_print("Loaded framebuffer");

    // TILL THIS !!!!!!!!!!

    // Arch depented code following !!!'
    // Both of these are located in "kcode/arch.h" that chooses arch and picks the right folder
    init_cpu();   // The stuff like GDT, IDT, TSS, etc so CPU
    init_memory(); // Similar to above, but PMM, VMM, HMM, etc. (because of cr3 and MMU)



    while (1)
        asm volatile("hlt");
}