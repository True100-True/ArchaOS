.section .text.entry
.global _start
.type _start,@function

_start:
    
    cli

    mov %rcx, %rdi

    andq $-16, %rsp
    subq $8, %rsp

    call kernel_main

.hang:
    hlt
    jmp .hang