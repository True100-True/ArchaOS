#ifndef PANIC_H
#define PANIC_H

#include "../lib/stdio.h"
#include "../lib/string.h"
#include <stdint.h>


void panic_ready(uint64_t sh, uint64_t sw);
void kernel_panic(const char* msg);

#endif