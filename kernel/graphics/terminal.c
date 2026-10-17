#include "graphics.h"

/*
Kernel primitive terminal
*/

font_t current_font = {0};

void term_init() { // , cozette_data *font
    if (graphics_init != true)
        return;
    cozette_data cozzete = load_glyphs();

    current_font = (font_t) {
        .glyphs      = cozzete.glyphs,
        .glyphsize   = cozzete.glyphsize,
        .glyph_count = cozzete.glyph_count,
        .width       = cozzete.width,
        .height      = cozzete.height
    };

}

// I want to remove this function and rewrite it into Putchar
/*
void putchar(char c) {
    serial_putchar(c); // inject into the output system to capture os output
    if (framebuffer_pb == NULL)
        return;

    if (c == '\r') {
        cursor_x = 0;
        return;
    }

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += current_font.height;

        if (cursor_y + current_font.height > framebuffer_pb->height)
            cursor_y = 0;

        return;
    }

    if (c == '\b') {
        if (cursor_x >= current_font.width) {
            cursor_x -= current_font.width;
        } else {
            cursor_x = 0;
        }

        // Erase previous character
        for (uint32_t row = 0;
             row < current_font.height;
             row++)
        {
            for (uint32_t col = 0;
                 col < current_font.width;
                 col++)
            {
                put_pixel(
                    cursor_x + col,
                    cursor_y + row,
                    0x00000000
                );
            }
        }

        return;
    }

    if (c == '\t') {
        cursor_x += current_font.width * 4;
        
        if (cursor_x + current_font.width > framebuffer_pb->width) {
            cursor_x = 0;
            cursor_y += current_font.height;
        }

        return;
    }

    uint32_t index = (uint8_t)c;

    if (index >= current_font.glyph_count)
        return;

    uint8_t *glyph =
        current_font.glyphs +
        index * current_font.glyphsize;

    uint32_t bytes =
        (current_font.width + 7) / 8;

    for (uint32_t row = 0;
         row < current_font.height;
         row++)
    {
        for (uint32_t col = 0;
             col < current_font.width;
             col++)
        {
            uint8_t byte =
                glyph[row * bytes + col / 8];

            if (byte & (0x80 >> (col % 8)))
            {
                put_pixel(
                    cursor_x + col,
                    cursor_y + row,
                    0xffffffff
                );
            }
        }
    }

    cursor_x += current_font.width;

    if (cursor_x + current_font.width > framebuffer_pb->width) {
        cursor_x = 0;
        cursor_y += current_font.height;
    }

    if (cursor_y + current_font.height > framebuffer_pb->height) {
        cursor_y = 0;
    }
}
*/

static void term_scroll(uint32_t background, font_t font) {
    if (framebuffer_pb == NULL)
        return;

    uint64_t pitch = framebuffer_pb->pitch;
    uint64_t height = framebuffer_pb->height;
    uint64_t line_height = font.height;

    if (line_height == 0 || line_height >= height)
        return;

    uint32_t *buffer = framebuffer_pb->address;
    memmove(
        buffer,
        buffer + line_height * pitch,
        (height - line_height) * pitch * sizeof(uint32_t)
    );
    uint64_t first_clear_row = height - line_height;
    for (uint64_t y = first_clear_row; y < height; y++) {
        uint32_t *row = buffer + y * pitch;
        for (uint64_t x = 0; x < pitch; x++) {
            row[x] = background;
        }
    }
}

static void ln(uint32_t background, font_t font) {
    cursor_x = 0;
    cursor_y += font.height;
    if (cursor_y + font.height > framebuffer_pb->height) {
        term_scroll(background, font);
        cursor_y = framebuffer_pb->height - font.height;
    }
}



void Putchar(char c, uint32_t foreground, uint32_t background, font_t font) {
    serial_putchar(c); // inject into the output system to capture os output
    if (framebuffer_pb == NULL)
        return;

    if (c == '\r') {
        cursor_x = 0;
        return;
    }

    if (c == '\n') {
        ln(background, font); // Aka \n
        return;
    }

    if (c == '\b') {
        if (cursor_x >= font.width) {
            cursor_x -= font.width;
        } else {
            cursor_x = 0;
        }

        /* Erase previous character */
        for (uint32_t row = 0;
             row < font.height;
             row++)
        {
            for (uint32_t col = 0;
                 col < font.width;
                 col++)
            {
                put_pixel(
                    cursor_x + col,
                    cursor_y + row,
                    background
                );
            }
        }

        return;
    }

    if (c == '\t') {
        cursor_x += font.width * 4;
        
        if (cursor_x + font.width > framebuffer_pb->width) {
            cursor_x = 0;
            cursor_y += font.height;
        }

        return;
    }

    uint32_t index = (uint8_t)c;

    if (index >= font.glyph_count)
        return;

    uint8_t *glyph =
        font.glyphs +
        index * font.glyphsize;

    uint32_t bytes =
        (font.width + 7) / 8;

    for (uint32_t row = 0;
         row < font.height;
         row++)
    {
        for (uint32_t col = 0;
             col < font.width;
             col++)
        {
            uint8_t byte =
                glyph[row * bytes + col / 8];

            if (byte & (0x80 >> (col % 8)))
            {
                put_pixel(
                    cursor_x + col,
                    cursor_y + row,
                    foreground
                );
            } 
            else 
            {
                put_pixel(
                    cursor_x + col,
                    cursor_y + row,
                    background
                );
            }
        }
    }

    cursor_x += font.width;

    if (cursor_x + font.width > framebuffer_pb->width) {
        term_scroll(background, font);
    }

    if (cursor_y + font.height > framebuffer_pb->height) {
        term_scroll(background, font);
    }
}


