#include "syscalls.h"

// Intel and AMD has same MSR's for syscall
#define MSR_EFER  0xC0000080
#define MSR_STAR  0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_FMASK 0xC0000084

/*
  // Enable IA32_EFER.SCE.
  write_msr(IA32_EFER, read_msr(IA32_EFER) | 1);
  // Set up the segments for syscall/sysret.
  uint64_t star = read_msr(IA32_STAR);
  star |= (uint64_t)KERNEL_BASE_SELECTOR << 32;
  star |= (uint64_t)USER_BASE_SELECTOR << 48;
  write_msr(IA32_STAR, star);
  write_msr(IA32_LSTAR, (uint64_t)&syscall_handler);
  write_msr(IA32_SFMASK, 0)
*/


void _init_syscalls(void) {
    /*
    //enable syscalls
    uint64_t efer = rdmsr(MSR_EFER);
    wrmsr(MSR_EFER, efer | (1ULL << 0));   // EFER.SCE

    // Set up entry
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);

    // other init
    uint64_t star =
        ((uint64_t)KERNEL_CS << 32) |
        ((uint64_t)(USER_CS - 16) << 48);
    wrmsr(MSR_STAR, star);

    /*
     * Clear IF, TF, and DF on syscall entry.
     * IF = bit 9, TF = bit 8, DF = bit 10.
     *//*
    wrmsr(MSR_FMASK, (1ULL << 9) | (1ULL << 8) | (1ULL << 10));

    */

      // Enable IA32_EFER.SCE.
    wrmsr(MSR_EFER, rdmsr(MSR_EFER) | 1);
    // Set up the segments for syscall/sysret.
    uint64_t star = rdmsr(MSR_STAR);
    star |= (uint64_t)KERNEL_CS << 32;
    star |= (uint64_t)USER_CS << 48;
    wrmsr(MSR_STAR, star);
    wrmsr(MSR_LSTAR, (uint64_t)&syscall_entry);
    wrmsr(MSR_FMASK, 0);
}

bool init_syscalls() {
    if (!has_cpu_msr())
        return false;
    
    
    // Check if CPUID is supported? why
    uint32_t max_ext = 0;
    __get_cpuid(0x8000000, &max_ext, NULL, NULL, NULL); 
    if (max_ext < 0x8000004) {
        ;
    }
    
    
    // We don't check for cpu right now, TODO
    _init_syscalls();
    
    return true;
}

void syscall_dispatch(void) {
    print("Called syscall!!"); // extremly ahh
    
    return;
}

