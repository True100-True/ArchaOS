.global divide_handler 
.global invalid_opcode_handler 
.global general_protection_handler 
.global page_fault_handler
.global double_fault_handler

.text

divide_handler:

    push %rax
    push %rbx
    push %rcx
    push %rdx

    call divide_error

    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    iretq

invalid_opcode_handler:
    push %rax
    push %rbx
    push %rcx
    push %rdx

    call invalid_opcode

    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    iretq

general_protection_handler:
    push %rax
    push %rbx
    push %rcx
    push %rdx

    call general_protection

    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    iretq

page_fault_handler:
    push %rax
    push %rbx
    push %rcx
    push %rdx

    call page_fault

    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    iretq

double_fault_handler:
    push %rax
    push %rbx
    push %rcx
    push %rdx

    call double_fault

    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    iretq