#include "syscalls.h"

#include <stdint.h> // For uint64_t, integer types
#include "../../../lib/string.h" // NULL
#include "../cpu/msr/msr.h" // For wrmsr and rdmsr
#include "../../../lib/stdbool.h" // for return type of bool
#include "../cpu/gdt/gdt.h" // For get_cs and get_ds


// This basicly controls everything not only applications do stuff but aswell as
// kernel configuration, kernel runtime configuration, scheduler stuff
// is begin set and modified by syscalls, SO its extremly important do have them

// Intel and AMD has same MSR's for syscall
#define MSR_EFER  0xC0000080
#define MSR_STAR  0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_FMASK 0xC0000084
static SYSCALL_HANDLER syscalls[SYSCALL_COUNT] = { 0 };

//// Basic syscalls (write protected, NOT removable OR configurable)


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

// TODO: Proper FLAGS check !!!

enum syscall_return assign_syscall_handler(SYSCALL_HANDLER handler) {
    if (handler.ID >= SYSCALL_COUNT || handler.function == 0) {
        return SYSCALL_INVALID;
    } else if (syscalls[handler.ID].FLAGS & SYSCALL_FLAG_EXISTS) {
        return SYSCALL_TAKEN;
    }
    // Follows the SYSCALL_HANDLER structure
    syscalls[handler.ID] = handler;
    return SYSCALL_SUCCESS;
}
enum syscall_return remove_syscall_handler(SYSCALL_HANDLER handler) {
    if (handler.ID >= SYSCALL_COUNT) {
        return SYSCALL_INVALID;
    }
    syscalls[handler.ID].FLAGS = 0; // 0 -> None existent / unassigned
    return SYSCALL_SUCCESS;
}

enum syscall_return assign_syscall(uint64_t id, syscall_fn function, uint64_t FLAGS) {
    if (id >= SYSCALL_COUNT || function == 0) {
        return SYSCALL_INVALID;
    } else if (syscalls[id].FLAGS & SYSCALL_FLAG_EXISTS) {
        return SYSCALL_TAKEN;
    }
    
    // Follows the SYSCALL_HANDLER structure
    syscalls[id].ID = id;
    syscalls[id].function = function;
    syscalls[id].FLAGS = FLAGS;

    return SYSCALL_SUCCESS;
}
enum syscall_return remove_syscall(uint64_t id) {
    if (id >= SYSCALL_COUNT) {
        return SYSCALL_INVALID;
    }
    syscalls[id].FLAGS = 0; // 0 -> None existent / unassigned
    return SYSCALL_SUCCESS;
}
// TODO: Make a syscall remove and assign with privilagues flags included

//// SYSCALL functions burned by default

// assign syscall wrapper
enum syscall_return assign_syscall_usermode(
    uint64_t ARG1, uint64_t ARG2, uint64_t ARG3,
uint64_t ARG4, uint64_t ARG5, uint64_t ARG6) {
    // ARG1 -> desired SYSCALL_HANDLER ID (pointer!!)
    // rest is ignored
    SYSCALL_HANDLER *sys_handler = (SYSCALL_HANDLER *)(uintptr_t)ARG1;
    return assign_syscall(sys_handler->ID, sys_handler->function, sys_handler->FLAGS);
}

// remove syscall wrapper
enum syscall_return remove_syscall_usermode(
    uint64_t ARG1, uint64_t ARG2, uint64_t ARG3,
uint64_t ARG4, uint64_t ARG5, uint64_t ARG6) {
    // ARG1 -> desired SYSCALL_HANDLER ID (pointer!!)
    // rest is ignored
    SYSCALL_HANDLER *sys_handler = (SYSCALL_HANDLER *)(uintptr_t)ARG1;
    return remove_syscall(sys_handler->ID);
}

// get information about X function
SYSCALL_HANDLER return_syscall_usermode(    
    uint64_t ARG1, uint64_t ARG2, uint64_t ARG3,
uint64_t ARG4, uint64_t ARG5, uint64_t ARG6) {
    // ARG1 -> desired SYSCALLs ID
    // If syscall ID is too big, return the 2047 ID :)
    // rest is ignored
    uint64_t syscall_id = ARG1;
    if (syscall_id > SYSCALL_COUNT)
        return syscalls[2048-1];
    return syscalls[syscall_id];
}


// TODO: properly check return statuses
void syscall_config_mode(
    uint64_t ARG1, uint64_t ARG2, uint64_t ARG3,
    uint64_t ARG4, uint64_t ARG5, uint64_t ARG6) {
    // Should not be configurable
    // When triggering syscall config mode, 3 services will appear
    // service 1. assign syscall (ID: 2046)
    // service 2. remove syscall (ID: 2047)
    // service 3. get syscall    (ID: 2048)
    // These services are forced into the syscall table
    // meaning if you have syscalls on this ID's already, back them up
    // service 3 returns every information about syscall, FLAGS, syscall_handler and the function pointer
    // No arguments

    //enum syscall_return status;
    
    // remove all existing syscalls occuping that space
    remove_syscall(2046);
    remove_syscall(2047);
    remove_syscall(2048);

    // Means: Assign syscall on ID 2046, function assign_syscall_usermode with flags: exists, only kernel can modify
    unsigned int flags = SYSCALL_FLAG_EXISTS | SYSCALL_FLAG_KERNEL_ONLY;
    assign_syscall(2046, (syscall_fn)assign_syscall_usermode, flags); // TODO: remove hardcoded values
    assign_syscall(2047, (syscall_fn)remove_syscall_usermode, flags);
    assign_syscall(2048, (syscall_fn)return_syscall_usermode, flags);
    return; // perfecto
}
// Parsing syscalls

struct syscall_frame {
    uint64_t ID;  // RAX
    uint64_t ARG1;  // RDI
    uint64_t ARG2;  // RSI
    uint64_t ARG3;  // RDX
    uint64_t ARG4;  // R10
    uint64_t ARG5;   // R8
    uint64_t ARG6;   // R9
};

uint64_t syscall_dispatch(struct syscall_frame *f) {
    if (f->ID > SYSCALL_COUNT || syscalls[f->ID].ID == 0 || syscalls[f->ID].FLAGS & SYSCALL_FLAG_EXISTS) {
        return SYSCALL_INVALID; // Work on return types and stuff
    }
    return syscalls[f->ID].function(f->ARG1, f->ARG2, f->ARG3, f->ARG4, f->ARG5, f->ARG6);
}

// SYSCALL initialize

bool init_syscalls() {
    if (!has_cpu_msr())
        return false;
    
    // Check if CPUID is supported? why
    /*
    uint32_t max_ext = 0;
    __get_cpuid(0x8000000, &max_ext, NULL, NULL, NULL); 
    if (max_ext < 0x8000004) {
        ;
    }*/

    // We don't check for cpu right now, TODO
    _init_syscalls();
    
    // Clear syscalls buffer
    memset(syscalls, 0, SYSCALL_COUNT);

    // assign configuration syscall
    // ID 2049 is reserved for configuration mode
    assign_syscall(2049, (syscall_fn)syscall_config_mode, 
        SYSCALL_FLAG_EXISTS | SYSCALL_FLAG_WRITE_PROTECTED);
    return true;
}

// TODO: add reserved ID's (now they are only in documentation only)