#ifndef FONT_H
#define FONT_H

// Strip down of cozzete

#include <stdint.h>
#include "../../../common/fonts/cozette.h"

typedef struct {
    uint8_t* glyphs;
    uint32_t glyphsize;
    uint32_t glyph_count;
    uint32_t width;
    uint32_t height;
} font_t;


static inline uint8_t* get_glyph_font(font_t* font, char c) {
    return font->glyphs + ((uint32_t)c * font->glyphsize);
}

#endif