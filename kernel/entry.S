.global _start
.type _start, @function

_start:
    mov %rcx, %rdi

    andq $-16, %rsp

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang