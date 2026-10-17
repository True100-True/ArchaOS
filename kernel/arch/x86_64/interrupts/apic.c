#include "apic.h"

#define IA32_APIC_BASE 0x1B

uint64_t apic_discover() {
    // L ACPI init
    uint64_t a = rdmsr(IA32_APIC_BASE);
    // 0:7 - reserved
    // 8 - BSP (bootstrap processor)
    // 9:10 - reserved (For APIC case)
    // 10 - x2APIC enable
    // 11 - x2APIC global enable
    // 11 - enable or disable ACPI for this processor
    // 12:31 - base address
    // 31:64 - reserved 
    return a;
}

// local APIC registers
/* More info about registers is at 
  Intel(R) 64 and IA-32 Architectures
    Software Developer’s Manual
  at
  Table 13-1. Local APIC Register Address Map

https://github.com/dreamportdev/Osdev-Notes/blob/master/02_Architecture/07_APIC.md
Spurious Interrupt Vector: offset 0xF0.
EOI (end of interrupt): offset 0xB0.
Timer LVT: offset 0x320.
Local APIC ID: offset 0x20.
*/

// FOLLOWING CODE USES MMIO APIC ACCESS !!!
#define APIC_SPURIOUS_INT_VECTOR 0xF0
#define APIC_EOI 0xB0
#define APIC_LVT_TIMER 0x320
#define APIC_TIMER_INITIAL_COUNT 0x380
#define APIC_TIMER_DIVIDE 0x3E0
#define APIC_ID 0x20
#define APIC_DIVIDE 0x3E0
#define APIC_INIT_COUNT 0x380

static uintptr_t apic_addr = 0;
static uintptr_t apic_read_reg(uintptr_t base, uint32_t reg) {
    return *(volatile uint32_t *)(base + reg);
}

static void apic_write_reg(uintptr_t base, uint32_t reg, uint32_t value) {
    *(volatile uint32_t *)(base + reg) = value;
}

void local_apic_init(uintptr_t apic_addr, bool enabled) {
    // let the apic_addr shadow the global variable
    print("Initializing local APIC..\n");
    print("\tEnabling..");
    if (!enabled) { // je [addr]
        uint64_t ia32_apic = rdmsr(IA32_APIC_BASE);
        ia32_apic |= (1ULL << 11);
        wrmsr(IA32_APIC_BASE, ia32_apic);
        print("OK (was disabled)\n");
    } else {
        print("OK (was enabled)\n");
    }

    //// Get ID 
    uint32_t apic_id_raw = apic_read_reg(apic_addr, APIC_ID);
    uint8_t apic_id = (apic_id_raw >> 24) & 0xFF;
    print("\tLocal APIC ID: %lx \n", (uint64_t)apic_id);

    //// Spurious vector

    uint32_t spurious = apic_read_reg(apic_addr, APIC_SPURIOUS_INT_VECTOR);
    
    print("\tSpurious handler: %lx \n", (uint64_t)spurious_int_handler);

    // enable Apic
    spurious |= (1 << 8);
    // disable focus proces check
    spurious &= ~(1 << 9);
    // Idt gate 40
    spurious = (spurious & 0xFFFFFF00) | 0xff;
    apic_write_reg(apic_addr, APIC_SPURIOUS_INT_VECTOR, spurious);
    idt_set_gate(0xff, spurious_int_handler);

    //// Timer

    uint32_t timer = apic_read_reg(apic_addr, APIC_LVT_TIMER);
    
    print("\tTimer handler: %lx \n", (uint64_t)tick_interrupt);
    /*
    // unmask (disable)
    timer &= ~(1 << 16);
    // Set to be periodic mode
    timer |= (1 << 17);
    timer &= ~(1 << 18);
    
    // idt gate 32
    timer = (timer & 0xFFFFFF00) | 32;
    apic_write_reg(apic_addr, APIC_LVT_TIMER, timer);
    idt_set_gate(32, tick_interrupt);
    */

    // Mask timer while configuring it.
    uint32_t lvt = 32;
    lvt |= 1u << 16;               // masked
    lvt |= 1u << 17;               // periodic
    apic_write_reg(apic_addr, APIC_LVT_TIMER, lvt);
    apic_write_reg(apic_addr, APIC_DIVIDE, 0xB); // divide by 1
    idt_set_gate(32, tick_interrupt);
}

void apic_timer_start(void) {
    uint32_t timer = apic_read_reg(apic_addr, APIC_LVT_TIMER);
    // unmask 16 to start clock
    timer &= ~(1u << 16);
    apic_write_reg(apic_addr, APIC_LVT_TIMER, timer);
    
}
void apic_timer_stop(void) {
    uint32_t timer = apic_read_reg(apic_addr, APIC_LVT_TIMER);
    // unmask 16 to stop clock
    timer |= (1u << 16);
    apic_write_reg(apic_addr, APIC_LVT_TIMER, timer);
}

void apic_eoi(void) {
    apic_write_reg(
        apic_addr,
        APIC_EOI,
        0
    );
}

void apic_start(void) {
    apic_write_reg(apic_addr, APIC_INIT_COUNT, 10000000);
}

// FOLLOWING CODE USES MSR TO ACCESS APIC
// Work in Progress

void local_apic_init_msr(uintptr_t apic_addr, bool enabled) {
    return; 
}

void apic_init() {
    print("[+] Loading APIC...\n");
    uint64_t a = apic_discover();
    
    bool is_bootstrap  = ((a >> 8) & 1)!=0;
    bool enabled       = ((a >> 11) & 1)!=0;
    bool x2apic        = ((a >> 10) & 1)!=0;
    uint64_t page = (a >> 12) & 0xFFFFFULL;
    uint64_t apic_base = page << 12;
    
    apic_addr = apic_base;
    print("APIC addr: %lx \n", (uint64_t)apic_base);

    print("Bootstrap processor: %u \n", (unsigned int)is_bootstrap);
    print("Enabled            : %u \n", (unsigned int)enabled);
    print("X2APIC availabled  : %u \n", (unsigned int)x2apic);

    if (!x2apic)
        local_apic_init(apic_addr, enabled);
    else 
        local_apic_init_msr(apic_addr, enabled); // Obviously wont work
    //print("APIC addr: %lx \n", addr);
    // Because we are in memory 1:1 mode (physical : virtual)
    // we can access that memory just find using a pointer
}
