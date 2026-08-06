#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
//#include "stdio.c"

void serial_init();
void serial_putchar(char c);
void serial_print(const char* str);
void serial_print_hex(uint64_t value);

#define NULL (void *)0

#endif