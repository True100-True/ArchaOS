#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "../lib/stdio.h"
#include "../lib/stdbool.h"

typedef struct {
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint32_t *address;
} framebuffer_t;

extern framebuffer_t *framebuffer_pb;
extern bool graphics_init;

void init_graphics(framebuffer_t *framebuffer);
void put_pixel(uint32_t x, uint32_t y, uint32_t color);

#endif