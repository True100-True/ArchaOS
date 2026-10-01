ENTRY(_start)

SECTIONS
{
    . = 0x100000;

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

    .kernel_stack (NOLOAD) : ALIGN(4K)
    {
        kernel_stack_bottom = .;
        . += 16K;
        kernel_stack_top = .;
    }
}
