#include "../kcode/arch.h"
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#include <stdarg.h>
#include <stdint.h>
#include "../graphics/graphics.h"

uint64_t cursor_x = 0;
uint64_t cursor_y = 0;

// This is the default font build into the kernel
// cozzete, cool font
// 
#include "../../common/fonts/cozette.h"

static const char hex_chars[] =
    "0123456789ABCDEF";

// THIS IS FOR QEMU ONLY !!! 
// DO NOT RUN on real HW with these enabled
// It can breake output or even damage something
// either way it is dangerous !!!
void serial_init()
{
    outb(0x00, 0x3F9); // Disable interrupts
    outb(0x80, 0x3FB); // Enable DLAB
    outb(0x03, 0x3F8); // Divisor low (38400 baud)
    outb(0x00, 0x3F9); // Divisor high
    outb(0x03, 0x3FB); // 8 bits, no parity, one stop bit
    outb(0xC7, 0x3FA); // Enable FIFO
    outb(0x0B, 0x3FC); // IRQs enabled, RTS/DSR
}
void serial_putchar(char c) {
    while (!(inb(0x3FD) & 0x20))
        ;

    outb(c, 0x3F8);
}
void serial_print(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        serial_putchar(str[i]);
        i++;
    }
}
void uint64_to_hex(uint64_t value, char* buffer)
{
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[18] = '\0';


    for(int i = 17; i >= 2; i--)
    {
        buffer[i] =
            hex_chars[value & 0xF];

        value >>= 4;
    }
}
void serial_print_hex(uint64_t value)
{
    char buffer[19];

    uint64_to_hex(value, buffer);

    serial_print(buffer);
    serial_putchar('\n');
}

static cozette_data current_font;

// VERY good low-copy print functions 
// There is string normalization, everything 
// and its good (atleast in my eyes)
// THIS is what is used in kernel, doesn't require much allocation
// kinda fast? I didn't check YET
void print_init() { // , cozette_data *font
    if (graphics_init =! true)
        return;
    current_font = load_glyphs();
}

void putchar(char c)
{
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

        /* Erase previous character */
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
static void print_uint(unsigned int value) {
    if (value >= 10)
        print_uint(value / 10);

    putchar('0' + (value % 10));
}
static void print_int(int value) {
    if (value < 0) {
        putchar('-');
        unsigned int n = 0u - (unsigned int)value;
        print_uint(n);
    } else {
        print_uint((unsigned int)value);
    }
}
static void print_hex(unsigned int value) {
    putchar('0');
    putchar('x');
    for (int i = 7; i >= 0; i--) {
        putchar(
            hex_chars[(value >> (i * 4)) & 0xF]
        );
    }
}
static void print_char(const char* to_print ) {
    while (*to_print) {
        putchar(*to_print);
        to_print++;
    }
}
void print(const char *format, ...) {
    if(framebuffer_pb == NULL)
        return;
    va_list args;
    va_start(args, format);

    while(*format) {
        if (*format != '%') {
            putchar(*format);
            format++;
            continue;
        }

        format++;

        if (!*format)
            break;
        
        char nxt_char = *(format+1);
        switch (nxt_char) {
            case 'd': {
                int value = va_arg(args, int);
                print_int(value);
                break;
            } case 'u': {
                unsigned int value = va_arg(args, unsigned int);
                print_uint(value);
                break;
            } case 's': {
                const char *value = va_arg(args, const char *);
                print_char(value); 
                break;
            } case 'x': {
                int value = va_arg(args, int);
                print_hex(value);
                break;
            } case '%': break;
            default:
                putchar('%');
                putchar(nxt_char);
                format += 2;
                continue; // idk how this behaves in switch (skips while OR switch)
        }
        format++;
    }
    va_end(args);
}

// NOT kernel friendly, requires TON of allocation
// kernel shouldn't take too much ram aswell
// so if print function takes 1GiB of normalized string its.. horrible
// So why are they here? I am planning on feature "run as kernel"
// horrible but good for testing (disabled by default in about.h)
// aswell some kernel level application will use this (probably)
// such as the kshell, krecovery, etc.
// Its mainly for extern not other  
static void append_char(char **out, size_t *remaining, char c) {
    if (*remaining <= 1)
        return;
    **out = c;
    (*out)++;
    (*remaining)--;
}
static void append_string(char **out, size_t *remaining, const char *str) {
    while (*str) {
        append_char(out, remaining, *str);
        str++;
    }
}
static void append_int(char **out, size_t *remaining, int value) {
    char tmp[12];
    int i = 0;

    unsigned int n;

    if (value < 0) {
        append_char(out, remaining, '-');
        n = 0u - (unsigned int)value;
    } else {
        n = (unsigned int)value;
    }

    if (n == 0) {
        append_char(out, remaining, '0');
        return;
    }

    while (n > 0) {
        tmp[i++] = '0' + (n % 10);
        n /= 10;
    }

    while (i > 0)
        append_char(out, remaining, tmp[--i]);
}
void format(char *buffer, size_t size, const char *fmt, ...) {
    char *out = buffer;
    size_t remaining = size;

    if (size == 0)
        return;

    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            append_char(&out, &remaining, *fmt);
            fmt++;
            continue;
        }
        fmt++;
        switch (*fmt) {
            case 'd': {
                int value = va_arg(args, int);
                append_int(&out, &remaining, value);
                break;
            }
            case 's': {
                const char *value = va_arg(args, const char *);
                append_string(&out, &remaining, value);
                break;
            }
            case '%':
                append_char(&out, &remaining, '%');
                break;
            default:
                append_char(&out, &remaining, '%');
                append_char(&out, &remaining, *fmt);
                break;
        }
        fmt++;
    }
    *out = '\0';

    va_end(args);
}

void print_hex_(unsigned int num) {
    print_hex(num);
}