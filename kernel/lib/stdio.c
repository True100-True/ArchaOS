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
//#include "../../common/fonts/cozette.h"
#include "../graphics/font/font.h"

static const char hex_chars[] =
    "0123456789ABCDEF";

// THIS IS FOR QEMU ONLY !!! 
// DO NOT RUN on real HW with these enabled
// It can breake output or even damage something
// either way it is dangerous !!!
#if defined(QEMU_TEST) && QEMU_TEST == 1
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

void serial_print_dec(uint64_t N)
{
    char str[22]; // -18446744073709551615\n
    int i = 0;
    int sign = N;
    if (N < 0)
        N = -N;
    while (N > 0) {
        str[i++] = N % 10 + '0';
      	N /= 10;
    } 
    if (sign < 0) {
        str[i++] = '-';
    }
    str[i] = '\0';
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
    serial_print(str);
}
#endif

// VERY good low-copy print functions 
// There is string normalization, everything 
// and its good (atleast in my eyes)
// THIS is what is used in kernel, doesn't require much allocation
// kinda fast? I didn't check YET
void print_init() { // , cozette_data *font
    if (graphics_init != true)
        return;
    term_init();
}

/*
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
static void print_u64(uint64_t value) { 
    char buf[20]; 
    int i = 0; 
    if (value == 0) { 
        putchar('0'); 
        return; 
    } 
    while (value) { 
        buf[i++] = '0' + (value % 10); 
        value /= 10; 
    } 
    while (i--) 
    putchar(buf[i]); 
}
static void print_u64_hex(uint64_t value) {
    putchar('0');
    putchar('x');
    for (int i = 15; i >= 0; i--) {
        putchar(
            hex_chars[(value >> (i * 4)) & 0xF]
        );
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

        switch (*format) {
            case 'd': {
                int value = va_arg(args, int);
                print_int(value);
                break;
            } case 'u': {
                unsigned int value = va_arg(args, unsigned int);
                print_uint(value);
                break;
            } case 'n': { 
                uint64_t value = va_arg(args, uint64_t);
                print_u64(value);
                break;
            } case 's': {
                const char *value = va_arg(args, const char *);
                print_char(value); 
                break;
            } case 'x': {
                int value = va_arg(args, int);
                print_hex(value);
                break;
            } case 'l': {
                if (format[1] == 'x') {
                    uint64_t value = va_arg(args, uint64_t);
                    print_u64_hex(value);
                    format += 2;
                } else {
                    putchar('%');
                    putchar('l');
                    format++;
                }
                break;
            } case '%': break;
            default:
                putchar('%');
                putchar(*format);
                format += 2;
                continue; // idk how this behaves in switch (skips while OR switch)
        }
        format++;
    }
    va_end(args);
}
*/

static void print_uint(unsigned int value) {
    if (value >= 10)
        print_uint(value / 10);

    Putchar('0' + (value % 10), 0xffffffff, 0x00102030, current_font);
}
static void print_int(int value) {
    if (value < 0) {
        Putchar('-', 0xffffffff, 0x00102030, current_font);
        unsigned int n = 0u - (unsigned int)value;
        print_uint(n);
    } else {
        print_uint((unsigned int)value);
    }
}
static void print_hex(unsigned int value) {
    Putchar('0', 0xffffffff, 0x00102030, current_font);
    Putchar('x', 0xffffffff, 0x00102030, current_font);
    for (int i = 7; i >= 0; i--) {
        Putchar(
            hex_chars[(value >> (i * 4)) & 0xF],
            0xffffffff, 0x00102030, current_font
        );
    }
}
static void print_char(const char* to_print ) {
    while (*to_print) {
        Putchar(*to_print, 0xffffffff, 0x00102030, current_font);
        to_print++;
    }
}
static void print_u64(uint64_t value) { 
    char buf[20]; 
    int i = 0; 
    if (value == 0) { 
        Putchar('0', 0xffffffff, 0x00102030, current_font); 
        return; 
    } 
    while (value) { 
        buf[i++] = '0' + (value % 10); 
        value /= 10; 
    } 
    while (i--) 
    Putchar(buf[i], 0xffffffff, 0x00102030, current_font); 
}
static void print_u64_hex(uint64_t value) {
    Putchar('0', 0xffffffff, 0x00102030, current_font);
    Putchar('x', 0xffffffff, 0x00102030, current_font);
    for (int i = 15; i >= 0; i--) {
        Putchar(
            hex_chars[(value >> (i * 4)) & 0xF],
            0xffffffff, 0x00102030, current_font
        );
    }
}

// You may read my code and ask youself
// why the fuck does he make a putchar function
// that can print colorfully just to blind it with fake values?
// this is a demo of the stdio.h its experimental
// after a while I will try to somehow make this work with ansii codes 
void print(const char *format, ...) {
    if(framebuffer_pb == NULL)
        return;
    va_list args;
    va_start(args, format);

    while(*format) {
        if (*format != '%') {
            Putchar(*format, 0xffffffff, 0x00102030, current_font); // default background and foreground
            format++;
            continue;
        }

        format++;

        if (!*format)
            break;

        switch (*format) {
            case 'd': {
                int value = va_arg(args, int);
                print_int(value);
                break;
            } case 'u': {
                unsigned int value = va_arg(args, unsigned int);
                print_uint(value);
                break;
            } case 'n': { 
                uint64_t value = va_arg(args, uint64_t);
                print_u64(value);
                break;
            } case 's': {
                const char *value = va_arg(args, const char *);
                print_char(value); 
                break;
            } case 'x': {
                int value = va_arg(args, int);
                print_hex(value);
                break;
            } case 'l': {
                if (format[1] == 'x') {
                    uint64_t value = va_arg(args, uint64_t);
                    print_u64_hex(value);
                    format += 2;
                } else {
                    Putchar('%', 0xffffffff, 0x00102030, current_font);
                    Putchar('l', 0xffffffff, 0x00102030, current_font);
                    format++;
                }
                break;
            } case '%': break;
            default:
                Putchar('%', 0xffffffff, 0x00102030, current_font);
                Putchar(*format, 0xffffffff, 0x00102030, current_font);
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
