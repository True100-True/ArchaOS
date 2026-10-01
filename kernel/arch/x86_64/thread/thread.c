#include "thread.h"

context_t *initial_context(uint8_t *kernel_stack, uint64_t stack_size, uint64_t entry_point) {
    uint64_t stack_top = (uint64_t)kernel_stack + stack_size;
    stack_top &= ~0xFULL;
    context_t *ctx = (context_t *)(stack_top - sizeof(context_t));

    *ctx = (context_t){0};

    ctx->rip = entry_point;
    ctx->cs = get_cs();
    ctx->rflags = 0x202;

    return ctx;
}

context_t *catch_current_context(context_t *context) {
    return context;
}

// MAIN callables

thread_t thread_create(uint64_t entry_point) {
    thread_t new_thread = {0};

    new_thread.kernel_stack = kmalloc(STACK_SIZE);

    new_thread.cpu_state = initial_context(
        new_thread.kernel_stack,
        STACK_SIZE,
        entry_point
    );

    new_thread.thread_status = READY;
    new_thread.next = NULL;

    return new_thread;
}

void thread_kill(thread_t *thread) {
    thread->thread_status = DEAD;
    // Mark it dead and scheduler will clean it up
}

// Scheduler helper 
void thread_switch(thread_t *current_thread, thread_t *next) {
    // Get current context and store it
    context_t *current_context = grab_context();
    current_thread->cpu_state = current_context;

    // Get context from switch_to and restore it
    context_t* next_context = next->cpu_state;
    restore_context(next_context);
}