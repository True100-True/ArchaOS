#ifndef _STDINT_H
#define _STDINT_H

/* Exact-width integer types */

/*

typedef signed char             int8_t;
typedef unsigned char           uint8_t;

typedef signed short            int16_t;
typedef unsigned short          uint16_t;

typedef signed int              int32_t;
typedef unsigned int            uint32_t;

typedef signed long long        int64_t;
typedef unsigned long long      uint64_t;

// Least-width integer types 

typedef int8_t                  int_least8_t;
typedef uint8_t                 uint_least8_t;

typedef int16_t                 int_least16_t;
typedef uint16_t                uint_least16_t;

typedef int32_t                 int_least32_t;
typedef uint32_t                uint_least32_t;

typedef int64_t                 int_least64_t;
typedef uint64_t                uint_least64_t;

// Fast integer types 

typedef int8_t                  int_fast8_t;
typedef uint8_t                 uint_fast8_t;

typedef int64_t                 int_fast16_t;
typedef uint64_t                uint_fast16_t;

typedef int64_t                 int_fast32_t;
typedef uint64_t                uint_fast32_t;

typedef int64_t                 int_fast64_t;
typedef uint64_t                uint_fast64_t;

// Pointer-sized integers

typedef signed long long        intptr_t;
typedef unsigned long long      uintptr_t;

// Greatest-width integers

typedef int64_t                 intmax_t;
typedef uint64_t                uintmax_t;

#define INT8_MIN    (-128)
#define INT8_MAX    127
#define UINT8_MAX   0xFF

#define INT16_MIN   (-32768)
#define INT16_MAX   32767
#define UINT16_MAX  0xFFFF

#define INT32_MIN   (-2147483647 - 1)
#define INT32_MAX   2147483647
#define UINT32_MAX  0xFFFFFFFFU

#define INT64_MIN   (-9223372036854775807LL - 1)
#define INT64_MAX   9223372036854775807LL
#define UINT64_MAX  0xFFFFFFFFFFFFFFFFULL

#define INTPTR_MIN  INT64_MIN
#define INTPTR_MAX  INT64_MAX
#define UINTPTR_MAX UINT64_MAX

#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MAX UINT64_MAX

*/

#include <stdint.h> // gcc is mad at me for making my own

#endif