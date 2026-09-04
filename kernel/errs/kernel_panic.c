#include "pnc.h"
#include "../config.h"

void panic_screen() {

}

// The panic function that uses the screen
void kernel_panic(const char* msg) {
    

    asm volatile("cli");
    while (1) {
        asm volatile("hlt");
    }
}