#include "string.h"

typedef uint64_t unaligned_u64
    __attribute__((may_alias, aligned(1)));

// Minor optimalization 
void *memcpy(void *dest, const void *src, size_t n) {
#if (!defined(X86_64) || X86_64 == 1) && QEMU_TEST == 0
    __asm__ volatile (
        "cld\n\t"
        "rep movsb"
        : "+D"(dest), "+S"(src), "+c"(n)
        :
        : "memory"
    );
    return dest;
#else
    void *ret = dest;
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (n >= 32) {
        *(unaligned_u64 *)(d +  0) = *(const unaligned_u64 *)(s +  0);
        *(unaligned_u64 *)(d +  8) = *(const unaligned_u64 *)(s +  8);
        *(unaligned_u64 *)(d + 16) = *(const unaligned_u64 *)(s + 16);
        *(unaligned_u64 *)(d + 24) = *(const unaligned_u64 *)(s + 24);
        d += 32;
        s += 32;
        n -= 32;
    }

    while (n >= 8) {
        *(unaligned_u64 *)d = *(const unaligned_u64 *)s;
        d += 8;
        s += 8;
        n -= 8;
    }

    while (n--) {
        *d++ = *s++;
    }

    return ret;
#endif
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

size_t strlen(const char *str) {
    size_t len = 0;
    if (str == NULL)
        return 0;
    while (*str++ != '\0')
        ++len;
    return len;
}

char* strrchr(const char *str, int c) {
    const char *last = NULL;
    if (str == NULL)
        return NULL;
    for (;;) {
        if (*str == (char)c)
            last = str;
        if (*str == '\0')
            break;
        ++str;
    }
    return (char *)last;
}

/*

TODO: Implement these

char *strstr(const char *haystack, const char *needle);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
*/
