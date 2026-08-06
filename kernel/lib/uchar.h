#ifndef _UCHAR_H
#define _UCHAR_H

#include <stdint.h>

#ifndef __CHAR16_TYPE__
typedef uint16_t char16_t;
#endif

#ifndef __CHAR32_TYPE__
typedef uint32_t char32_t;
#endif

typedef uint8_t char8_t;
typedef uint64_t char64_t;

#endif