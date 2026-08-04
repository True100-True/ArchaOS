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




void serial_putchar(char c)
{
    outb(0x3F8, c);
}

void serial_print(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        serial_putchar(str[i]);
        i++;
    }
}
void serial_print_hex(uint64_t value)
{
    const char hex_chars[] = {
        '0','1','2','3','4','5','6','7',
        '8','9','A','B','C','D','E','F',
        0
    };

    char buffer[19];

    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[18] = 0;

    for (int i = 17; i >= 2; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }

    serial_print(buffer);
    serial_putchar('\n');
}


void kernel_crash(void) {
    asm volatile("ud2");
    while (1)
        asm volatile("hlt");
}

void kernel_panic(const char* reason) {
    serial_print("(panic) kernel paniced: ");
    serial_print(reason);
    serial_print("\n\n");
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
    
    while (1)
        asm volatile("hlt");
}