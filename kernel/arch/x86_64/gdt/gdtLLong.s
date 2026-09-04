.intel_syntax noprefix

.global load_gdt

.section .data

gdtr:
    .word 0
    .quad 0


.section .text

load_gdt:
    mov [gdtr], di
    mov [gdtr + 2], rsi

    lgdt [gdtr]

    call reload_segments

    ret


reload_segments:

    push 0x08
    lea rax, [rip + .reload_cs]
    push rax
    lretq


.reload_cs:

    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret