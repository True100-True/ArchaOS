#ifndef KMALLOC_H
#define KMALLOC_H

#include "stdio.h"

void kmalloc_init(char *start, int length);

void *kmalloc(int length);
void kfree(void *ptr);

#endif