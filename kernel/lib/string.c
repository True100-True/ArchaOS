#include "string.h"

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (n--)
        *d++ = *s++;

    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d < s) {
        while (n--)
            *d++ = *s++;
    } else {
        d += n;
        s += n;

        while (n--)
            *--d = *--s;
    }

    return dest;
}

void *memset(void *ptr, int value, size_t n) {
    unsigned char *p = ptr;

    while (n--)
        *p++ = (unsigned char)value;

    return ptr;
}

int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *x = a;
    const unsigned char *y = b;

    while (n--) {
        if (*x != *y)
            return *x - *y;
        x++;
        y++;
    }

    return 0;
}

char *strcpy(char *dest, const char *src) {
    char *start = dest;

    while ((*dest++ = *src++));

    return start;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }

    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && (*a == *b)) {
        a++;
        b++;
        n--;
    }

    if (n == 0)
        return 0;

    return (unsigned char)*a - (unsigned char)*b;
}

char *strchr(const char *str, int c) {
    while (*str) {
        if (*str == (char)c)
            return (char *)str;
        str++;
    }

    return (c == '\0') ? (char *)str : NULL;
}

/*

TODO: Implement these

char *strrchr(const char *str, int c);
char *strstr(const char *haystack, const char *needle);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
size_t strlen(const char *str);
*/