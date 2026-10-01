#include "msr.h"

uint64_t rdmsr(uint32_t msr) {
    uint32_t low;
    uint32_t high;

    asm volatile (
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );

    return ((uint64_t)high << 32) | low;
}
void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);

    asm volatile (
        "wrmsr"
        :
        : "c"(msr), "a"(low), "d"(high)
        : "memory"
    );
}

#define CPUID_FEATURE_MSR (1u << 5)
bool has_cpu_msr(void) {
    struct cpuid_result r = cpuid_(0, 0);
    if (r.eax < 1)
        return false;
    return true;
}

