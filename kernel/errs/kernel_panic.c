#include "../lib/stdio.h"


void kernel_panic(const char* msg) {
    serial_print("(panic) Kernel panic because ring-0 threw exception...\n\t");
    serial_print(msg);
    /*
    in future draw smth and try to recover as much as possible
    */

    asm volatile("cli");

    while (1)
        asm volatile("hlt");
}