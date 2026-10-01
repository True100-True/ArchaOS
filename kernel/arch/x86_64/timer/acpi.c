#include "acpi.h"

#define IA32_APIC_BASE 0x1B

uint32_t apic_discover() {
    // L ACPI init
    uint64_t a = rdmsr(IA32_APIC_BASE);
    // 0:7 - reserved
    // 8 - BSP (bootstrap processor)
    // 9:10 - reserved
    // 11 - enable or disable ACPI for this processor
    // 12:31 - base address
    // 31:64 - reserved 
    bool is_bootstrap = (a >> 8) & 1;
    bool enabled = (a >> 11) & 1;
    uint32_t apic_base = (a >> 12) & 0xFFFFF;
    return apic_base;
}

void apic_init() {
    uint64_t addr = (uint64_t)apic_discover();
    print("APIC addr: %llx \n", addr);
}
