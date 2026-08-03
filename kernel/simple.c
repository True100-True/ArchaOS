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
#include "fromboot.h"
#include <stdint.h>



static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

void serial_putchar(char c)
{
    outb(0x3F8, c);
}

/*
void serial_print(const char* str) {
    while (str++) {
        char c;
        serial_putchar();
    }
}
*/

void kernel_main(BootInfo* boot)
{
    serial_putchar('K');
    serial_putchar('R');
    serial_putchar('N');
    serial_putchar('L');

    serial_putchar(' ');

    if (boot == 0 || boot->magic == 0xCAFEBABEDEADBEEF)
    {
        serial_putchar('!');
        return;
    }

    if (boot->framebuffer_base == 0)
    {
        serial_putchar('F'); // framebuffer missing
    }
    else
    {
        serial_putchar('f'); // framebuffer OK
    }

    if (boot->width == 0 || boot->height == 0)
    {
        serial_putchar('S'); // size missing
    }
    else
    {
        serial_putchar('s'); // size OK
    }

    while (1)
        asm volatile("hlt");
}