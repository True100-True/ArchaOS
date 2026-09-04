#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include "../graphics/graphics.h"

// ---- QEMU TEST ONLY -----
#if !defined(QEMU_TEST) || QEMU_TEST == 1
void serial_init();
void serial_putchar(char c);
void serial_print(const char* str);
void serial_print_hex(uint64_t value);
#else
static inline void serial_init(void) {}
static inline void serial_putchar(char c) { (void)c; }
static inline void serial_print(const char *str) { (void)str; }
static inline void serial_print_hex(uint64_t value) { (void)value; }
#endif
// Remove with #ifndef NOT_QEMU_TEST !!!!!!!!

extern uint64_t cursor_x;
extern uint64_t cursor_y;

void print_init();
void putchar(char c);
void print(const char *format, ...);
void print_hex_(unsigned int value); // OBSOLETE THIS FUNCTION IN NEAR FUTURE

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif