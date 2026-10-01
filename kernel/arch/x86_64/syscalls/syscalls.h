#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h> // For uint64_t, integer types
#include "../../../lib/string.h" // NULL
#include "../../../lib/stdio.h" // print
#include "../cpu/msr/msr.h" // For wrmsr and rdmsr
#include "../../../lib/stdbool.h" // for return type of bool
#include "../cpu/gdt/gdt.h" // For get_cs and get_ds
#include "../cpu/cpuid/cpuid.h"

/* because I need dynamic drivers there will syscalls such as 'hook driver on..'*/

// Syscall ASM entry
extern void syscall_entry(void);

// Load up syscalls into the msr tables
bool init_syscalls();

#endif // SYSCALLS_H
