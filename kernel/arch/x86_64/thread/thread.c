#include "thread.h"

thread_t thread_create() {
    thread_t new_thread;
    new_thread.cpu_state = grab_context();
    new_thread.thread_status = READY;

    new_thread.kernel_stack = 0;
    new_thread.next = 0;
    return new_thread; // Scheduler will fill out missing fields and push to execution
}

void thread_kill(thread_t *thread) {
    thread->cpu_state = DEAD;
    // Mark it dead and scheduler will clean it up
}

// Scheduler helper 
void thread_switch(thread_t *current_thread, thread_t *next) {
    // Get current context and store it
    context_t *current_context = grab_context();
    current_thread->cpu_state = &current_context;

    // Get context from switch_to and restore it
    context_t* next_context = next->cpu_state;
    restore_context(next_context);
}