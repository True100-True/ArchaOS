.intel_syntax noprefix

.global load_idt


load_idt:

    mov [idtr], di
    mov [idtr+2], rsi

    lidt [idtr]

    ret


.section .data

idtr:
    .word 0
    .quad 0