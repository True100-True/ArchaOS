#ifndef COZZETE_H
#define COZZETE_H

#include <stdint.h>
#include "cozette_fnt.h"

/*
typedef struct {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charsize;
} PSF1_Header;


typedef struct {
    uint8_t* glyphs;
    uint8_t size;
} cozette_data;

*/

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
    uint32_t size;
    uint32_t width;
    uint32_t height;
} cozette_data;


cozette_data load_glyphs()
{
    PSF2_Header* header = (PSF2_Header*)cozette_psf;

    if (header->magic != 0x864AB572)
    {
        return (cozette_data){0};
    }

    cozette_data cd;

    cd.glyphs = cozette_psf + header->headersize;
    cd.size = header->glyphsize;
    cd.width = header->width;
    cd.height = header->height;

    return cd;
}

#endif // COZZETE_H
