#include "graphics.h"

framebuffer_t *framebuffer_pb = NULL;
bool graphics_init = false;

void init_graphics(framebuffer_t *framebuffer) {
    framebuffer_pb = framebuffer;
    graphics_init = true;
}


// TODO: Better / stronger graphics manipulation API
// I don't necessarly need a enrich graphics library
// For the sake of developers that would need it tho
void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (framebuffer_pb == NULL)
        return;

    if (x >= framebuffer_pb->width ||
        y >= framebuffer_pb->height)
        return;

    framebuffer_pb->address[
        (uint64_t)y * framebuffer_pb->pitch + x
    ] = color;
}