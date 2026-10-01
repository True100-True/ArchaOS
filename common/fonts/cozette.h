#ifndef COZZETE_H
#define COZZETE_H

#include <stdint.h>
#include "cozette_fnt.h"


#define PSF2_MAGIC 0x864AB572


typedef struct {

    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;

    uint32_t length;
    uint32_t glyphsize;

    uint32_t height;
    uint32_t width;

} PSF2_Header;



typedef struct {

    uint8_t* glyphs;

    uint32_t glyphsize;
    uint32_t glyph_count;

    uint32_t width;
    uint32_t height;

} cozette_data;



static inline uint8_t* get_glyph(
    cozette_data* font,
    char c
)
{
    return font->glyphs +
        ((uint32_t)c * font->glyphsize);
}



static inline cozette_data load_glyphs()
{
    PSF2_Header* header =
        (PSF2_Header*)cozette_psf;


    if(header->magic != PSF2_MAGIC)
    {
        return (cozette_data){0};
    }


    cozette_data cd;


    cd.glyphs =
        cozette_psf +
        header->headersize;


    cd.glyphsize =
        header->glyphsize;


    cd.glyph_count =
        header->length;


    cd.width =
        header->width;


    cd.height =
        header->height;


    return cd;
}


#endif