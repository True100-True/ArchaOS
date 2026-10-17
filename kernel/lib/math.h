#ifndef MATH_H
#define MATH_H

/* -- MATH function for C --*/

static inline int abs(int x) {
    return x < 0 ? -x : x;
}

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

#endif 