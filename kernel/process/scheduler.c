#include "scheduler.h"
#include "../kcode/arch.h"

// This is very architecture depended (in my opion)
// so I will keep a copy of scheduler in every arch

/*
// This code is retired until I will fix my kernel
static thread_t *current_thread = NULL;
static thread_t *threads_list = NULL;

context_t *schedule(context_t *context)
{
    if (current_thread == NULL) {
        thread_t *t = threades_list;

        while (t != NULL) {
            if (t->thread_status == READY) {
                current_thread = t;
                current_thread->thread_status = RUNNING;
                return current_thread->cpu_state;
            }

            t = t->next;
        }

        return NULL;
    }

    // Save current CPU state
    current_thread->cpu_state = context;

    if (current_thread->thread_status == RUNNING)
        current_thread->thread_status = READY;

    thread_t *next = current_thread->next;

    if (next == NULL)
        next = threades_list;

    thread_t *start = next;

    do {
        if (next->thread_status == READY) {
            current_thread = next;
            current_thread->thread_status = RUNNING;

            return current_thread->cpu_state;
        }

        next = next->next;

        if (next == NULL)
            next = threades_list;

    } while (next != start);

    // Nobody else is ready.
    current_thread->thread_status = RUNNING;

    return current_thread->cpu_state;
}

void scheduler() {
    for (;;) {

    }
}
*/