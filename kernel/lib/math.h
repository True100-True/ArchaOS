#ifndef MATH_H
#define MATH_H

#include "../config.h"

/* -- MATH function for C --*/

static inline int abs(int x) {
    return x < 0 ? -x : x;
}

#if defined(x86_64) || X86_64 == 1
static inline double sin(double x) {
    double result;
    asm("fsin" : "=t" (result) : "0" (x));
    return result;
}

static inline double cos(double x) {
    double result;
    asm("fcos" : "=t"(result) : "0"(x));
    return result;
}
#else

#endif

#endif 