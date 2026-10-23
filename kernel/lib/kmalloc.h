#ifndef KMALLOC_H
#define KMALLOC_H

#include "stdio.h"
#include "../kcode/arch.h"

#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

// basic kmalloc

void kmalloc_init(void);

void *kmalloc(uint64_t size);
void kfree(void *ptr);
void *kcalloc(unsigned int num_of_items, size_t len_of_items); // Implementaion of calloc (very poor?)

#endif