#ifndef CPUID_H
#define CPUID_H

#include <stdint.h>
#include <cpuid.h>

struct cpuid_result {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};

static inline struct cpuid_result cpuid_(uint32_t leaf, uint32_t subleaf) {
    struct cpuid_result r;

    asm volatile (
        "cpuid"
        : "=a"(r.eax),
          "=b"(r.ebx),
          "=c"(r.ecx),
          "=d"(r.edx)
        : "a"(leaf),
          "c"(subleaf)
    );

    return r;
}

#endif