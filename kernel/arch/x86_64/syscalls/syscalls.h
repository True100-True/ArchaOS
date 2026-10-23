#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h> // For uint64_t, integer types
#include "../../../lib/stdbool.h"

typedef uint64_t (*syscall_fn)(
    uint64_t, uint64_t, uint64_t,
    uint64_t, uint64_t, uint64_t
);
#define SYSCALL_COUNT 2048

// Each syscall has each of these
typedef struct SYSCALL_HANDLER {
    uint64_t ID;
    uint64_t FLAGS;
    syscall_fn function;    
} SYSCALL_HANDLER;

enum syscall_return {
    SYSCALL_SUCCESS,
    SYSCALL_ERROR,
    SYSCALL_VALID,
    SYSCALL_INVALID,
    SYSCALL_TAKEN,
    SYSCALL_NOT_PRIVILAGED
};

enum syscall_flags {
    SYSCALL_FLAG_EXISTS          = (1u << 0),
    SYSCALL_FLAG_KERNEL_ONLY     = (1u << 1),
    SYSCALL_FLAG_USER_ONLY       = (1u << 2),
    SYSCALL_FLAG_WRITE_PROTECTED = (1u << 3),
};

/* because I need dynamic drivers there will syscalls such as 'hook driver on..'*/

// Syscall ASM entry
extern void syscall_entry(void);

// Load up syscalls into the msr tables
bool init_syscalls();

// Manipulate with syscalls
enum syscall_return assign_syscall(uint64_t id, syscall_fn function, uint64_t FLAGS);
enum syscall_return remove_syscall(uint64_t id);

#endif // SYSCALLS_H
