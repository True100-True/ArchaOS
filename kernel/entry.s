.section .text.entry
.global _start
.type _start,@function

_start:
    
    cli

    mov %rcx, %rdi
    lea kernel_stack_top(%rip), %rsp
    andq $-16, %rsp
    subq $8, %rsp

    call kernel_main

.hang:
    hlt
    jmp .hang
