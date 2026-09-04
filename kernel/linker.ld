ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    . = ALIGN(4K);
    kernel_stack_bottom = .;
    . += 16K;  /* or whatever stack size you want */
    . = ALIGN(4K);
    kernel_stack_top = .;

    .text :
    {
        KEEP(*(.text.entry))
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(COMMON)
        *(.bss*)
    }
}