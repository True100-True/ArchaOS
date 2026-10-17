#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <stdint.h>
#include "../lib/stdio.h"
#include "../lib/stdbool.h"
#include "font/font.h"
//#include "../../common/fonts/cozette.h"
#include "../lib/math.h"
#include "../lib/string.h"

//#define FRAMEBUFFER_VIRT 0x0000000000000000ULL

typedef struct {
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint32_t *address;
} framebuffer_t;

extern framebuffer_t *framebuffer_pb;
extern bool graphics_init;

extern font_t current_font;

/*
typedef struct {
    uint64_t r;
    uint64_t g;
    uint64_t b;
} color_t;
*/

void init_graphics(framebuffer_t *framebuffer);
void put_pixel(uint32_t x, uint32_t y, uint32_t color);
void draw_line(uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color);
void Putchar(char c, uint32_t foreground, uint32_t background, font_t font);
void fill_rect(uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color);
void term_init();

void put_pixel_q(framebuffer_t *buffer, uint32_t x, uint32_t y, uint32_t color);
void draw_line_q(framebuffer_t *buffer, uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color);
void fill_rect_q(framebuffer_t *buffer, uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, uint32_t color);


//// TTY aswell

#endif