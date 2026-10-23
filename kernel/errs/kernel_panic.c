#include "pnc.h"

void panic_screen() {
    //framebuffer_pb->address[];
    fill_rect(framebuffer_pb->width/2+100, framebuffer_pb->height/2+50, framebuffer_pb->width/2-100, framebuffer_pb->height/2-50, 0xFFFF8C15);
}

// The panic function that uses the screen
void kernel_panic(const char* msg) {
    serial_print("Paniced: \n\t");
    serial_print(msg);
    panic_screen();
    asm volatile("cli");
    while (1) {
        asm volatile("hlt");
    }
}