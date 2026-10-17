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

void put_pixel_q(framebuffer_t *buffer, uint32_t x, uint32_t y, uint32_t color) {
    if (buffer == NULL)
        buffer = framebuffer_pb;
    if (framebuffer_pb == NULL)
        return;

    if (x >= buffer->width ||
        y >= buffer->height)
        return;

    buffer->address[
        (uint64_t)y * buffer->pitch + x
    ] = color;
}

/*
void draw_rectangle(uint32_t x, uint32_t y, uint32_t size, uint32_t color) {
    // Size is size of one thing

    put_pixel();
}
    */

/*
IBM's draw line algorithm 
or Bre

plotLine(x0, y0, x1, y1)
    dx = x1 - x0
    dy = y1 - y0
    D = 2*dy - dx
    y = y0

    for x from x0 to x1
        plot(x, y)
        if D > 0
            y = y + 1
            D = D + (2 * (dy - dx))
        else
            D = D + 2*dy
        end if
*/
void draw_line(uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color) {
    int x0 = (int)sx;
    int y0 = (int)sy;
    int x1 = (int)dx;
    int y1 = (int)dy;

    int deltaX = abs(x1 - x0);
    int stepX = x0 < x1 ? 1 : -1;

    int deltaY = -abs(y1 - y0);
    int stepY = y0 < y1 ? 1 : -1;

    int error = deltaX + deltaY;

    while (1) {
        put_pixel((uint32_t)x0, (uint32_t)y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int error2 = 2 * error;

        if (error2 >= deltaY) {
            error += deltaY;
            x0 += stepX;
        }

        if (error2 <= deltaX) {
            error += deltaX;
            y0 += stepY;
        }
    }
}

void draw_line_q(framebuffer_t *buffer, uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color) {
    int x0 = (int)sx;
    int y0 = (int)sy;
    int x1 = (int)dx;
    int y1 = (int)dy;

    int deltaX = abs(x1 - x0);
    int stepX = x0 < x1 ? 1 : -1;

    int deltaY = -abs(y1 - y0);
    int stepY = y0 < y1 ? 1 : -1;

    int error = deltaX + deltaY;

    while (1) {
        put_pixel_q(buffer, (uint32_t)x0, (uint32_t)y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int error2 = 2 * error;

        if (error2 >= deltaY) {
            error += deltaY;
            x0 += stepX;
        }

        if (error2 <= deltaX) {
            error += deltaX;
            y0 += stepY;
        }
    }
}

void fill_rect(uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color) {
    // Draw a rectangle with source x,y and destivation x,y
    uint32_t ix;
    uint32_t iy;
    for (ix = sx; ix < dx; ix++) {
        for (iy = sy; iy < dy; iy++) {
            put_pixel(ix, iy, color);
        }
    }
}

void fill_rect_q(framebuffer_t *buffer, uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color) {
    // Draw a rectangle with source x,y and destivation x,y
    uint32_t ix;
    uint32_t iy;
    for (ix = sx; ix < dx; ix++) {
        for (iy = sy; iy < dy; iy++) {
            put_pixel_q(buffer, ix, iy, color);
        }
    }
}



/*
void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    
}
*/

// TTY
