# 0 "entry.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "entry.S"
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
