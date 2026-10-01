.global divide_handler
.global invalid_opcode_handler
.global general_protection_handler
.global page_fault_handler
.global double_fault_handler

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

divide_handler:
    SAVE_REGISTERS
    call divide_error
    RESTORE_REGISTERS
    iretq

invalid_opcode_handler:
    SAVE_REGISTERS
    call invalid_opcode
    RESTORE_REGISTERS
    iretq

/* #DF, #GP and #PF place an error code below the hardware iret frame. */
general_protection_handler:
    SAVE_REGISTERS
    call general_protection
    RESTORE_REGISTERS
    add $8, %rsp
    iretq

page_fault_handler:
    SAVE_REGISTERS
    call page_fault
    RESTORE_REGISTERS
    add $8, %rsp
    iretq

double_fault_handler:
    SAVE_REGISTERS
    call double_fault
    RESTORE_REGISTERS
    add $8, %rsp
    iretq
