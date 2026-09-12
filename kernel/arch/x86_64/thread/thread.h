#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "../../../lib/string.h"
#include "../../../lib/kmalloc.h"
#include "../mm/vmm.h"

// Credit: https://baponkar.github.io/Osdev-Notes/05_Scheduling/02_Scheduler.html

typedef struct {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
} context_t;

typedef enum {
    READY,
    RUNNING,
    DEAD,
    ASLEEP
} status_t;

typedef struct thread_t {
    status_t thread_status;
    uint8_t *kernel_stack;
    context_t *cpu_state;
    struct thread_t* next;
} thread_t;

// ASM functions

extern context_t *grab_context();
extern void restore_context(context_t *ctx);
extern uint8_t get_ds(void);
extern uint8_t get_cs(void);

thread_t thread_create(uint64_t entry_point);
void thread_kill(thread_t *thread);
void thread_switch(thread_t *current_thread, thread_t *switch_to);

context_t *catch_current_context(context_t *context);

#endif // THREAD_H