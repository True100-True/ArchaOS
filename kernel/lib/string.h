#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *ptr, int value, size_t n);
int memcmp(const void *a, const void *b, size_t n);
char *strcpy(char *dest, const char *src);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strchr(const char *str, int c);

/*
TODO: Implement these

char *strrchr(const char *str, int c);
char *strstr(const char *haystack, const char *needle);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
size_t strlen(const char *str);

*/

#endif