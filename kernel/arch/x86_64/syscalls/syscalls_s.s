.intel_syntax noprefix

.global syscall_entry

.text

.macro SAVE_REGISTERS
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
.endm

.macro RESTORE_REGISTERS
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
.endm

syscall_entry:
    # eventually save user registers
    # switch to kernel stack
    # call syscall_dispatch 
    # restore state

    swapgs

    mov     QWORD PTR gs:8, rsp
    mov     rsp, QWORD PTR gs:0

    SAVE_REGISTERS

    call syscall_dispatch

    RESTORE_REGISTERS # Users' registers
    
    mov     rsp, QWORD PTR gs:8

    swapgs
    sysretq