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
}