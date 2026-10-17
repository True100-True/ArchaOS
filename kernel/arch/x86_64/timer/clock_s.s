.intel_syntax noprefix

.global tick_interrupt
.extern clock_interrupt

.text

.macro SAVE_REGISTERS
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
.endm

.macro RESTORE_REGISTERS
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
.endm

tick_interrupt:
    cld
    SAVE_REGISTERS

    mov rbx, rsp
    and rsp, -16
    call clock_interrupt
    mov rsp, rbx

    RESTORE_REGISTERS
    iretq

