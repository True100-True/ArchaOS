# 0 "entry.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "entry.S"
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
